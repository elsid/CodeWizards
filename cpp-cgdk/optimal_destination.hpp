#pragma once

#include "world_graph.hpp"
#include "context.hpp"
#include "helpers.hpp"

namespace strategy {

enum class TowerNumber {
    FIRST,
    SECOND,
    UNKNOWN,
};

class TowersOrder {
public:
    TowersOrder(const model::World& world, model::Faction faction);

    model::LaneType get_lane(const model::Building& unit) const;
    TowerNumber get_number(const model::Building& unit) const;
    const Point& get_enemy_tower(model::LaneType lane, TowerNumber number) const;

private:
    model::Faction faction_;
    std::array<std::array<Point, 2>, 3> friend_towers_;
    std::array<std::array<Point, 2>, 3> enemy_towers_;

    model::LaneType get_lane(const model::Building& unit, const std::array<std::array<Point, 2>, 3>& towers) const;
    TowerNumber get_number(const model::Building& unit, const std::array<Point, 2>& towers) const;
};

bool is_immortal(const Context& context, const model::Building& unit);

class GetNodeScore {
public:
    static constexpr double ENEMY_WIZARD_REDUCE_FACTOR = 16.0;
    static constexpr double ENEMY_MINION_REDUCE_FACTOR = 2.0;
    static constexpr double ENEMY_TOWER_REDUCE_FACTOR = 400.0;
    static constexpr double ENEMY_BASE_REDUCE_FACTOR = 800.0;
    static constexpr double FRIEND_WIZARD_REDUCE_FACTOR = 8.0;
    static constexpr double PATH_LENGTH_REDUCE_FACTOR = 0.2;
    static constexpr double FRIEND_TOWER_MULT_FACTOR = 1.0;
    static constexpr double FRIEND_BASE_MULT_FACTOR = 2.0;
    static constexpr double FRIEND_MINION_MULT_FACTOR = 1.0;
    static constexpr double WIZARD_DAMAGE_PROBABILITY = 0.25;
    static constexpr double WIZARD_ELIMINATION_PROBABILITY = 0.05;

    struct NodeInfo {
        double enemy_wizards_weight = 0;
        double enemy_minions_weight = 0;
        double enemy_towers_weight = 0;
        double enemy_immortal_towers_weight = 0;
        double friend_wizards_weight = 0;
        double friend_minions_weight = 0;
        double friend_towers_weight = 0;
        double bonus_weight = 0;
        double enemy_base_weight = 0;
        double enemy_immortal_base_weight = 0;
        double friend_base_weight = 0;
        WorldGraph::Path path_from_me;
        WorldGraph::Path path_from_friend_base;

        void add_other(const model::Unit&, double) {}

        void add_other(const model::Bonus&, double weight) {
            bonus_weight = weight;
        }

        void add_enemy(const Context&, const model::Unit&, double) {}

        void add_enemy(const Context&, const model::Wizard&, double weight) {
            enemy_wizards_weight += weight;
        }

        void add_enemy(const Context&, const model::Minion&, double weight) {
            enemy_minions_weight += weight;
        }

        void add_enemy(const Context& context, const model::Building& unit, double weight) {
            if (unit.getType() == model::BUILDING_FACTION_BASE) {
                if (is_immortal(context, unit)) {
                    enemy_immortal_base_weight = weight;
                } else {
                    enemy_base_weight = weight;
                }
            } else {
                if (is_immortal(context, unit)) {
                    enemy_immortal_towers_weight += weight;
                } else {
                    enemy_towers_weight += weight;
                }
            }
        }

        void add_friend(const model::Unit&, double) {}

        void add_friend(const model::Wizard&, double weight) {
            friend_wizards_weight += weight;
        }

        void add_friend(const model::Minion&, double weight) {
            friend_minions_weight += weight;
        }

        void add_friend(const model::Building& unit, double weight) {
            if (unit.getType() == model::BUILDING_FACTION_BASE) {
                friend_base_weight = weight;
            } else {
                friend_towers_weight += weight;
            }
        }
    };

    GetNodeScore(const Context& context, const WorldGraph& graph, model::LaneType target_lane, const model::Wizard& wizard);

    double operator ()(WorldGraph::Node node) const;

    const std::vector<NodeInfo>& nodes_info() const {
        return nodes_info_;
    }

    int get_tower_number(const model::Building& unit) const;

private:
    const Context& context_;
    const WorldGraph& graph_;
    model::LaneType target_lane_;
    const model::Wizard& wizard_;
    std::vector<NodeInfo> nodes_info_;
    WorldGraph::Node wizard_nearest_node_;

    template <class Unit>
    void fill_nodes_info(const typename std::unordered_map<UnitId, CachedUnit<Unit>>& units) {
        for (const auto& v : units) {
            const auto& unit = v.second.value();
            const auto nearest_node = get_nearest_node(graph_.nodes(), get_position(unit));
            const auto distance_to_nearest = get_position(unit).distance(nearest_node.second);
            for (const auto& node : graph_.nodes()) {
                const auto distance = get_position(unit).distance(node.second);
                if (distance < 2 * wizard_.getVisionRange()) {
                    const auto distance_weight = distance_to_nearest / (distance ? distance : 1.0);
                    auto& node_info = nodes_info_[node.first];
                    if (unit.getFaction() == model::FACTION_ACADEMY || unit.getFaction() == model::FACTION_RENEGADES) {
                        if (unit.getFaction() == wizard_.getFaction()) {
                            node_info.add_friend(unit, distance_weight);
                        } else {
                            node_info.add_enemy(context_, unit, distance_weight);
                        }
                    } else {
                        node_info.add_other(unit, distance_weight);
                    }
                }
            }
        }
    }

    double high_life_score(WorldGraph::Node node, const NodeInfo& node_info) const;
    double low_life_score(WorldGraph::Node node, const NodeInfo& node_info) const;
    double low_life_score_single(const NodeInfo& node_info) const;
};

struct GetLaneScore {
    const Context& context;
    const WorldGraph& graph;
    const GetNodeScore& get_node_score;

    double operator ()(model::LaneType lane) const;
    double get_lane_length(model::LaneType lane) const;
};

WorldGraph::Pair get_nearest_node(const WorldGraph::Nodes& nodes, const Point& position);
std::array<double, model::_LANE_COUNT_> get_lanes_scores(const Context& context, const WorldGraph& graph, const model::Wizard& wizard);
WorldGraph::Node get_optimal_destination(const Context& context, const WorldGraph& graph, model::LaneType target_lane, const model::Wizard& wizard);

}
