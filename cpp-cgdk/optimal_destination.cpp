#include "optimal_destination.hpp"
#include "optimal_target.hpp"

#include <algorithm>

#ifdef ELSID_STRATEGY_DEBUG

#include "debug/output.hpp"

#include <iostream>

#endif

namespace strategy {

TowersOrder::TowersOrder(const model::World& world, model::Faction faction)
    : faction_(faction),
      friend_towers_({{
        {{Point(1370.66, 3650),
          Point(2312.13, 3950)}}, // TOP
        {{Point(902.613, 2768.1),
          Point(1929.29, 2400)}}, // MIDDLE
        {{Point(50, 2693.26),
          Point(350, 1656.75)}}, // BOTTOM
      }}),
      enemy_towers_({{
        {{Point(world.getWidth() - 1370.66, world.getHeight() - 3650),
          Point(world.getWidth() - 2312.13, world.getHeight() - 3950)}}, // TOP
        {{Point(world.getWidth() - 902.613, world.getHeight() - 2768.1),
          Point(world.getWidth() - 1929.29, world.getHeight() - 2400)}}, // MIDDLE
        {{Point(world.getWidth() - 50, world.getHeight() - 2693.26),
          Point(world.getWidth() - 350, world.getHeight() - 1656.75)}}, // BOTTOM
      }}) {
}

model::LaneType TowersOrder::get_lane(const model::Building& unit) const {
    if (unit.getFaction() == faction_) {
        return get_lane(unit, friend_towers_);
    } else {
        return get_lane(unit, enemy_towers_);
    }
}

TowerNumber TowersOrder::get_number(const model::Building& unit) const {
    const auto lane = get_lane(unit);
    if (unit.getFaction() == faction_) {
        return get_number(unit, friend_towers_[lane]);
    } else {
        return get_number(unit, enemy_towers_[lane]);
    }
}

const Point& TowersOrder::get_enemy_tower(model::LaneType lane, TowerNumber number) const {
    return enemy_towers_[lane][std::size_t(number)];
}

model::LaneType TowersOrder::get_lane(const model::Building& unit, const std::array<std::array<Point, 2>, 3>& towers) const {
    const auto lane = std::find_if(towers.begin(), towers.end(),
        [&] (const auto& v) { return this->get_number(unit, v) != TowerNumber::UNKNOWN; });
    if (lane == towers.end()) {
        std::ostringstream error;
        error << "Lane not found for tower with id " << unit.getId()
              << " in " << __PRETTY_FUNCTION__ << " at " << __FILE__ << ":" << __LINE__;
        throw std::logic_error(error.str());
    }
    return model::LaneType(lane - towers.begin());
}

TowerNumber TowersOrder::get_number(const model::Building& unit, const std::array<Point, 2>& towers) const {
    const auto number = std::find_if(towers.begin(), towers.end(),
        [&] (const auto& v) { return get_position(unit).distance(v) < 100; });
    return number == towers.end() ? TowerNumber::UNKNOWN : TowerNumber(number - towers.begin());
}

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

GetNodeScore::GetNodeScore(const Context &context, const WorldGraph &graph, model::LaneType target_lane, const model::Wizard& wizard)
        : context_(context),
          graph_(graph),
          target_lane_(target_lane),
          wizard_(wizard),
          nodes_info_(graph.nodes().size()),
          wizard_nearest_node_(get_nearest_node(graph.nodes(), get_position(wizard)).first) {
    fill_nodes_info<model::Bonus>(get_units<model::Bonus>(context_.cache()));
    fill_nodes_info<model::Building>(get_units<model::Building>(context_.cache()));
    fill_nodes_info<model::Minion>(get_units<model::Minion>(context_.cache()));
    fill_nodes_info<model::Wizard>(get_units<model::Wizard>(context_.history_cache()));

    for (const auto& node : graph.nodes()) {
        auto& node_info = nodes_info_[node.first];
        node_info.path_from_me = graph.get_shortest_path(wizard_nearest_node_, node.first);
        node_info.path_from_friend_base = graph.get_shortest_path(graph.friend_base(), node.first);
    }
}

double GetNodeScore::operator ()(WorldGraph::Node node) const {
    const auto& node_info = nodes_info_.at(node);
    const auto mean_life_change = get_units<model::Wizard>(context_.cache()).at(context_.self().getId()).mean_life_change_speed();

    if ((mean_life_change >= 0 || (mean_life_change < 0 && - context_.self().getLife() / mean_life_change >= 100))
            && context_.self().getLife() > context_.self().getMaxLife() / 2) {
        return high_life_score(node, node_info);
    } else {
        return low_life_score(node_info);
    }
}

double GetNodeScore::high_life_score(WorldGraph::Node node, const NodeInfo& node_info) const {
    const auto reduce_factor = 1.0 / (
                node_info.enemy_minions_weight * ENEMY_MINION_REDUCE_FACTOR
                + node_info.enemy_wizards_weight * ENEMY_WIZARD_REDUCE_FACTOR
                + node_info.enemy_towers_weight * ENEMY_TOWER_REDUCE_FACTOR
                + node_info.enemy_base_weight * ENEMY_BASE_REDUCE_FACTOR
                + node_info.friend_minions_weight * FRIEND_MINION_REDUCE_FACTOR
                + node_info.friend_wizards_weight * FRIEND_WIZARD_REDUCE_FACTOR
                + (1 + node_info.path_from_me.length) * PATH_LENGTH_REDUCE_FACTOR
            );

    const auto bonus_score = node_info.bonus_weight
            * context_.game().getBonusScoreAmount();

    if ((target_lane_ != model::_LANE_UNKNOWN_ && !graph_.lanes_nodes().at(target_lane_).count(node))
            || wizard_nearest_node_ == node) {
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

double GetNodeScore::low_life_score(const NodeInfo& node_info) const {
    const auto sum = std::accumulate(node_info.path_from_me.nodes.begin(), node_info.path_from_me.nodes.end(), 0.0,
        [&] (auto sum, auto v) { return sum + this->low_life_score_single(nodes_info_.at(v)); });

    return sum + low_life_score_single(node_info) - node_info.path_from_me.nodes.size() / 2
            - node_info.path_from_friend_base.nodes.size() / 3;
}

double GetNodeScore::low_life_score_single(const NodeInfo& node_info) const {
    return - node_info.enemy_minions_weight
            - node_info.enemy_wizards_weight
            - node_info.enemy_towers_weight
            - node_info.enemy_base_weight;
}

double GetLaneScore::operator ()(model::LaneType lane) const {
    const auto& lane_nodes = graph.lanes_nodes().at(lane);
    const auto node_enemy_weight = [&] (auto node) {
        const auto& node_info = get_node_score.nodes_info().at(node);
        return 4 * node_info.enemy_wizards_weight + node_info.enemy_minions_weight + 2 * node_info.enemy_towers_weight;
    };
    const auto enemy_weight = std::accumulate(lane_nodes.begin(), lane_nodes.end(), std::numeric_limits<double>::min(),
        [&] (auto sum, auto node) { return sum + node_enemy_weight(node); });
    const auto friend_weight = std::accumulate(lane_nodes.begin(), lane_nodes.end(), std::numeric_limits<double>::min(),
        [&] (auto sum, auto node) {
            const auto& node_info = get_node_score.nodes_info().at(node);
            return sum + 4 * node_info.friend_wizards_weight + node_info.friend_minions_weight + 2 * node_info.friend_towers_weight;
        });
    const auto max_enemy = *std::max_element(lane_nodes.begin(), lane_nodes.end(), [&] (auto lhs, auto rhs) {
        return node_enemy_weight(lhs) < node_enemy_weight(rhs);
    });
    const auto lane_length = get_lane_length(lane);
    const auto to_firend_base = graph.get_shortest_path(max_enemy, graph.friend_base()).length;
    const auto factor = to_firend_base > lane_length / 2
            ? bounded_line_factor(to_firend_base, lane_length / 2, lane_length)
            : bounded_line_factor(to_firend_base, lane_length / 2, 0);
    return enemy_weight / friend_weight * (1 + factor);
}

double GetLaneScore::get_lane_length(model::LaneType lane) const {
    switch (lane) {
        case model::LANE_BOTTOM:
        case model::LANE_TOP:
            return 2 * context.game().getMapSize();
        case model::LANE_MIDDLE:
            return std::sqrt(2.0) * context.game().getMapSize();
        default:
            return 0;
    }
}

std::array<double, model::_LANE_COUNT_> get_lanes_scores(const Context& context, const WorldGraph& graph, const model::Wizard& wizard) {
    const GetNodeScore get_node_score(context, graph, model::_LANE_UNKNOWN_, wizard);
    const GetLaneScore get_lane_score {context, graph, get_node_score};
    const auto lanes = {model::LANE_TOP, model::LANE_MIDDLE, model::LANE_BOTTOM};
    std::array<double, model::_LANE_COUNT_> lanes_score = {{0, 0, 0}};
    std::transform(lanes.begin(), lanes.end(), lanes_score.begin(), [&] (auto lane) { return get_lane_score(lane); });
    return lanes_score;
}

WorldGraph::Node get_optimal_destination(const Context& context, const WorldGraph& graph, model::LaneType target_lane, const model::Wizard& wizard) {
    if (target_lane == model::_LANE_UNKNOWN_) {
        const auto lanes_score = get_lanes_scores(context, graph, wizard);
        target_lane = model::LaneType(std::max_element(lanes_score.begin(), lanes_score.end()) - lanes_score.begin());
    }
    const GetNodeScore get_node_score(context, graph, target_lane, wizard);
    std::vector<double> scores;
    scores.reserve(graph.nodes().size());
    std::transform(graph.nodes().begin(), graph.nodes().end(), std::back_inserter(scores),
        [&] (const auto& v) { return get_node_score(v.first); });
    return WorldGraph::Node(std::max_element(scores.begin(), scores.end()) - scores.begin());
}

}
