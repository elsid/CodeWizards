#include "optimal_destination.hpp"
#include "optimal_target.hpp"

#include <algorithm>

#ifdef STRATEGY_DEBUG

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

static WorldGraph::Pair get_nearest_node_by_path(const WorldGraph& graph, const std::vector<WorldGraph::Pair>& nodes, const WorldGraph::Node node) {
    std::unordered_map<WorldGraph::Node, double> path_lengths;
    std::transform(nodes.begin(), nodes.end(), std::inserter(path_lengths, path_lengths.end()),
        [&] (const auto& v) { return std::make_pair(v.first, graph.get_shortest_path(node, v.first).length); });
    return *std::min_element(nodes.begin(), nodes.end(),
        [&] (const auto& lhs, const auto& rhs) { return path_lengths.at(lhs.first) < path_lengths.at(rhs.first); });
}

static WorldGraph::Path get_path_to_nearest_node(const WorldGraph& graph, const std::vector<WorldGraph::Pair>& nodes, const WorldGraph::Node node) {
    std::vector<WorldGraph::Path> paths;
    std::transform(nodes.begin(), nodes.end(), std::inserter(paths, paths.end()),
        [&] (const auto& v) { return graph.get_shortest_path(node, v.first); });
    return *std::min_element(paths.begin(), paths.end(),
        [&] (const auto& lhs, const auto& rhs) { return lhs.length < rhs.length; });
}

GetNodePenalty::GetNodePenalty(const Context& context, const WorldGraph& graph, model::LaneType target_lane)
        : context_(context),
          graph_(graph),
          target_lane_(target_lane),
          nodes_info_(graph.nodes().size()),
          self_nearest_node_(get_nearest_node(graph.nodes(), get_position(context.self())).first) {
    fill_nodes_info<model::Building>();
    fill_nodes_info<model::Minion>();
    fill_nodes_info<model::Wizard>();

    for (const auto& node : graph.nodes()) {
        auto& node_info = nodes_info_[node.first];
        node_info.path = graph.get_shortest_path(self_nearest_node_, node.first);
        node_info.distance_to_friend_base = node.second.distance(graph.nodes().at(graph.friend_base()));
        node_info.distance_to_enemy_base = node.second.distance(graph.nodes().at(graph.enemy_base()));
    }

    max_path_length_ = std::max_element(nodes_info_.begin(), nodes_info_.end(),
        [] (const auto& lhs, const auto& rhs) { return lhs.path.length < rhs.path.length; })->path.length;

    max_distance_to_friend_base_ = std::max_element(nodes_info_.begin(), nodes_info_.end(),
        [] (const auto& lhs, const auto& rhs) {
            return lhs.distance_to_friend_base < rhs.distance_to_friend_base;
    })->distance_to_friend_base;

    max_distance_to_enemy_base_ = std::max_element(nodes_info_.begin(), nodes_info_.end(),
        [] (const auto& lhs, const auto& rhs) {
            return lhs.distance_to_enemy_base < rhs.distance_to_enemy_base;
    })->distance_to_enemy_base;
}

double GetNodePenalty::operator ()(WorldGraph::Node node) const {
    if (target_lane_ != model::_LANE_UNKNOWN_ && !graph_.lanes_nodes().at(target_lane_).count(node)) {
        return MAX_PENALTY;
    }
    const auto& node_info = nodes_info_.at(node);
    if (node_info.enemies_count == 0 && node_info.friends_count == 0) {
        return MAX_PENALTY;
    }
    const auto parties_diff = node_info.friends_count - node_info.enemies_count;
    const auto parties_penalty = 0.5 * double(parties_diff) / double(enemies_and_friends_count_) + 1.0;
    const auto path_length_penalty = node_info.path.length / max_path_length_;
    const auto distance_to_friend_base_penalty = node_info.distance_to_friend_base / max_distance_to_friend_base_;
    const auto distance_to_enemy_base_penalty = node_info.distance_to_enemy_base / max_distance_to_enemy_base_;
    return parties_penalty * PARTIES_PENALTY_WEIGHT
            + path_length_penalty * PATH_LENGTH_PENALTY_WEIGHT
            + distance_to_friend_base_penalty * DISTANCE_TO_FRIEND_BASE_PENALTY_WEIGHT
            + distance_to_enemy_base_penalty * DISTANCE_TO_ENEMY_BASE_PENALTY_WEIGHT;
}

WorldGraph::Node get_optimal_destination(const Context& context, const WorldGraph& graph, model::LaneType target_lane) {
//    GetNodePenalty get_node_penalty(context, graph, target_lane);
//    std::vector<double> penalties;
//    penalties.reserve(graph.nodes().size());
//    std::transform(graph.nodes().begin(), graph.nodes().end(), std::back_inserter(penalties),
//        [&] (const auto& v) { return get_node_penalty(v.first); });
//    return WorldGraph::Node(std::min_element(penalties.begin(), penalties.end()) - penalties.begin());

    const auto has_near_bonus = [&] (const auto& node) {
        return has_near_units(node.second, context.world().getBonuses(), graph.zone_size());
    };

    const auto is_enemy = [&] (const auto& unit) {
        return strategy::is_enemy(unit, context.self().getFaction());
    };

    const auto enemy_buildings = filter_units(context.world().getBuildings(), is_enemy);
    const auto enemy_minions = filter_units(context.world().getMinions(), is_enemy);
    const auto enemy_wizards = filter_units(context.world().getWizards(), is_enemy);

    const auto has_near_enemy_buildings = [&] (const auto& node) {
        return has_near_units(node.second, enemy_buildings, graph.zone_size());
    };

    const auto has_near_enemy_minions = [&] (const auto& node) {
        return has_near_units(node.second, enemy_minions, graph.zone_size());
    };

    const auto has_near_enemy_wizards = [&] (const auto& node) {
        return has_near_units(node.second, enemy_wizards, graph.zone_size());
    };

    const auto has_near_enemy = [&] (const auto& node) {
        return has_near_enemy_buildings(node) || has_near_enemy_minions(node) || has_near_enemy_wizards(node);
    };

    const auto at_target_lane = [&] (const auto& node) {
        return target_lane == model::_LANE_UNKNOWN_ || graph.lanes_nodes().at(target_lane).count(node.first);
    };

    const auto nodes_with_bonuses = filter_nodes(graph.nodes(), has_near_bonus);
    const auto nodes_with_enemy = filter_nodes(graph.nodes(),
        [&] (const auto& node) { return at_target_lane(node) && has_near_enemy(node); });

    const auto nearest_node = get_nearest_node(graph.nodes(), get_position(context.self())).first;

    if (!nodes_with_bonuses.empty() && !nodes_with_enemy.empty()) {
        const auto with_enemy = get_nearest_node_by_path(graph, nodes_with_enemy, nearest_node).first;
        if (with_enemy == nearest_node) {
            return with_enemy;
        }
    }

    if (!nodes_with_bonuses.empty()) {
        return get_nearest_node_by_path(graph, nodes_with_bonuses, nearest_node).first;
    }

    if (!nodes_with_enemy.empty()) {
        const auto to_friend_base = get_path_to_nearest_node(graph, nodes_with_enemy, graph.friend_base());
        const auto to_enemy_base = graph.get_shortest_path(nearest_node, graph.enemy_base());
        if (to_friend_base.length < 2 * graph.zone_size() && 2 * graph.zone_size() < to_enemy_base.length) {
            return to_friend_base.nodes.back();
        }
        return get_nearest_node_by_path(graph, nodes_with_enemy, nearest_node).first;
    }

    return graph.center();
}

}
