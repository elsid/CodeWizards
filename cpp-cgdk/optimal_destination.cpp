#include "optimal_destination.hpp"
#include "optimal_target.hpp"

#include <algorithm>

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

static WorldGraph::Pair get_nearest_node(const WorldGraph::Nodes& nodes, const Point& position) {
    return *std::min_element(nodes.begin(), nodes.end(),
        [&] (const auto& lhs, const auto& rhs) { return position.distance(lhs.first) < position.distance(rhs.first); });
}

static WorldGraph::Pair get_nearest_node_by_path(const WorldGraph& graph, const std::vector<WorldGraph::Pair>& nodes, const WorldGraph::Node node) {
    std::unordered_map<WorldGraph::Node, double> path_lengths;
    std::transform(nodes.begin(), nodes.end(), std::inserter(path_lengths, path_lengths.end()),
        [&] (const auto& v) { return std::make_pair(v.first, graph.get_shortest_path(node, v.first).length); });
    return *std::min_element(nodes.begin(), nodes.end(),
        [&] (const auto& lhs, const auto& rhs) { return path_lengths[lhs.first] < path_lengths[rhs.first]; });
}

static WorldGraph::Path get_path_to_nearest_node(const WorldGraph& graph, const std::vector<WorldGraph::Pair>& nodes, const WorldGraph::Node node) {
    std::vector<WorldGraph::Path> paths;
    std::transform(nodes.begin(), nodes.end(), std::inserter(paths, paths.end()),
        [&] (const auto& v) { return graph.get_shortest_path(node, v.first); });
    return *std::min_element(paths.begin(), paths.end(),
        [&] (const auto& lhs, const auto& rhs) { return lhs.length < rhs.length; });
}

WorldGraph::Node get_optimal_destination(const Context& context, const WorldGraph& graph, model::LaneType target_lane) {
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
        return target_lane != model::_LANE_UNKNOWN_ && graph.lanes_nodes().at(target_lane).count(node.first);
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
