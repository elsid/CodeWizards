#include "optimal_destination.hpp"
#include "optimal_target.hpp"

#include <algorithm>

#ifdef ELSID_STRATEGY_DEBUG

#include "debug/output.hpp"

#include <iostream>

#endif

namespace strategy {

template <class Predicate>
std::vector<WorldGraph::Pair> filter_nodes(const WorldGraph::Nodes& nodes, const Predicate& predicate) {
    std::vector<WorldGraph::Pair> result;
    result.reserve(nodes.size());
    std::copy_if(nodes.begin(), nodes.end(), std::back_inserter(result), predicate);
    return result;
}

template <class T>
bool has_near_units(const Point& position, const std::vector<T>& units, double distance) {
    return units.end() != std::find_if(units.begin(), units.end(),
        [&] (const auto& unit) { return position.distance(get_position(unit)) < distance; });
}

template <class T>
bool has_near_units(const Point& position, const std::vector<const T*>& units, double distance) {
    return units.end() != std::find_if(units.begin(), units.end(),
        [&] (auto unit) { return position.distance(get_position(*unit)) < distance; });
}

WorldGraph::Pair get_nearest_node(const WorldGraph::Nodes& nodes, const Point& position) {
    return *std::min_element(nodes.begin(), nodes.end(),
        [&] (const auto& lhs, const auto& rhs) { return position.distance(lhs.second) < position.distance(rhs.second); });
}

GetNodeScore::GetNodeScore(const Context &context, const WorldGraph &graph, model::LaneType target_lane)
        : context_(context),
          graph_(graph),
          target_lane_(target_lane),
          nodes_info_(graph.nodes().size()),
          self_nearest_node_(get_nearest_node(graph.nodes(), get_position(context.self())).first) {
    fill_nodes_info<model::Bonus>();
    fill_nodes_info<model::Building>();
    fill_nodes_info<model::Minion>();
    fill_nodes_info<model::Wizard>();

    for (const auto& node : graph.nodes()) {
        auto& node_info = nodes_info_[node.first];
        node_info.path = graph.get_shortest_path(self_nearest_node_, node.first);
    }
}

double GetNodeScore::operator ()(WorldGraph::Node node) const {
    const auto& node_info = nodes_info_.at(node);

    const auto reduce_factor = 1.0 / (
                node_info.enemy_minions_weight * ENEMY_MINION_REDUCE_FACTOR
                + node_info.enemy_wizards_weight * ENEMY_WIZARD_REDUCE_FACTOR
                + node_info.enemy_towers_weight * ENEMY_TOWER_REDUCE_FACTOR
                + node_info.enemy_base_weight * ENEMY_BASE_REDUCE_FACTOR
                + node_info.friend_minions_weight * FRIEND_MINION_REDUCE_FACTOR
                + node_info.friend_wizards_weight * FRIEND_WIZARD_REDUCE_FACTOR
                + (1 + node_info.path.length) * PATH_LENGTH_REDUCE_FACTOR
            );

    const auto bonus_score = node_info.bonus_weight
            * context_.game().getBonusScoreAmount();

    if (target_lane_ != model::_LANE_UNKNOWN_ && !graph_.lanes_nodes().at(target_lane_).count(node)) {
        return bonus_score * reduce_factor;
    }

    const auto mult_factor = 1.0
            + node_info.friend_towers_weight * FRIEND_TOWER_MULT_FACTOR
            + node_info.friend_base_weight * FRIEND_BASE_MULT_FACTOR;

    const auto enemy_minions_score = node_info.enemy_minions_weight
            * (context_.game().getMinionDamageScoreFactor() + context_.game().getMinionEliminationScoreFactor())
            * context_.game().getMinionLife();

    const auto enemy_wizards_score = node_info.enemy_wizards_weight
            * (context_.game().getWizardDamageScoreFactor() * WIZARD_DAMAGE_PROBABILITY
               + context_.game().getWizardEliminationScoreFactor() * WIZARD_ELIMINATION_PROBABILITY)
            * context_.game().getWizardBaseLife();

    const auto enemy_towers_score = node_info.enemy_towers_weight
            * (context_.game().getBuildingDamageScoreFactor() + context_.game().getBuildingEliminationScoreFactor())
            * context_.game().getGuardianTowerLife();

    const auto enemy_base_score = node_info.enemy_base_weight
            * context_.game().getVictoryScore();

    return (1 + enemy_minions_score + enemy_wizards_score + enemy_towers_score + enemy_base_score + bonus_score)
            * reduce_factor * mult_factor;
}

WorldGraph::Node get_optimal_destination(const Context& context, const WorldGraph& graph, model::LaneType target_lane) {
    const GetNodeScore get_node_score(context, graph, target_lane);
    std::vector<double> scores;
    scores.reserve(graph.nodes().size());
    std::transform(graph.nodes().begin(), graph.nodes().end(), std::back_inserter(scores),
        [&] (const auto& v) { return get_node_score(v.first); });
    return WorldGraph::Node(std::max_element(scores.begin(), scores.end()) - scores.begin());
}

}
