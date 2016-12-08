#ifndef STRATEGY_OPTIMAL_DESTINATION_HPP
#define STRATEGY_OPTIMAL_DESTINATION_HPP

#include "world_graph.hpp"
#include "context.hpp"
#include "optimal_target.hpp"
#include "optimal_position.hpp"

namespace strategy {

class GetNodeScore {
public:
    static constexpr const double ENEMY_WIZARD_REDUCE_FACTOR = 16.0;
    static constexpr const double ENEMY_MINION_REDUCE_FACTOR = 2.0;
    static constexpr const double ENEMY_TOWER_REDUCE_FACTOR = 400.0;
    static constexpr const double ENEMY_BASE_REDUCE_FACTOR = 800.0;
    static constexpr const double FRIEND_WIZARD_REDUCE_FACTOR = 8.0;
    static constexpr const double FRIEND_MINION_REDUCE_FACTOR = 1.0;
    static constexpr const double PATH_LENGTH_REDUCE_FACTOR = 0.25;
    static constexpr const double FRIEND_TOWER_MULT_FACTOR = 1.0;
    static constexpr const double FRIEND_BASE_MULT_FACTOR = 2.0;
    static constexpr const double WIZARD_DAMAGE_PROBABILITY = 0.25;
    static constexpr const double WIZARD_ELIMINATION_PROBABILITY = 0.05;

    GetNodeScore(const Context& context, const WorldGraph& graph, model::LaneType target_lane);

    double operator ()(WorldGraph::Node node) const;

private:
    struct NodeInfo {
        double enemy_wizards_weight = 0;
        double enemy_minions_weight = 0;
        double enemy_towers_weight = 0;
        double friend_wizards_weight = 0;
        double friend_minions_weight = 0;
        double friend_towers_weight = 0;
        double bonus_weight = 0;
        double enemy_base_weight = 0;
        double friend_base_weight = 0;
        WorldGraph::Path path;

        void add_other(const model::Unit&, double) {}

        void add_other(const model::Bonus&, double weight) {
            bonus_weight = weight;
        }

        void add_enemy(const model::Unit&, double) {}

        void add_enemy(const model::Wizard&, double weight) {
            enemy_wizards_weight += weight;
        }

        void add_enemy(const model::Minion&, double weight) {
            enemy_minions_weight += weight;
        }

        void add_enemy(const model::Building& unit, double weight) {
            if (unit.getType() == model::BUILDING_FACTION_BASE) {
                enemy_base_weight = weight;
            } else {
                enemy_towers_weight += weight;
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

    const Context& context_;
    const WorldGraph& graph_;
    model::LaneType target_lane_;
    std::vector<NodeInfo> nodes_info_;
    WorldGraph::Node self_nearest_node_;

    template <class Unit>
    void fill_nodes_info() {
        for (const auto& v : get_units<Unit>(context_.cache())) {
            const auto& unit = v.second.value();
            const auto nearest_node = get_nearest_node(graph_.nodes(), get_position(unit));
            const auto distance_to_nearest = get_position(unit).distance(nearest_node.second);
            for (const auto& node : graph_.nodes()) {
                const auto distance = get_position(unit).distance(node.second);
                if (distance < context_.self().getVisionRange()) {
                    const auto distance_weight = distance_to_nearest / (distance ? distance : 1.0);
                    auto& node_info = nodes_info_[node.first];
                    if (unit.getFaction() == model::FACTION_ACADEMY || unit.getFaction() == model::FACTION_RENEGADES) {
                        if (unit.getFaction() == context_.self().getFaction()) {
                            node_info.add_friend(unit, distance_weight);
                        } else {
                            node_info.add_enemy(unit, distance_weight);
                        }
                    } else {
                        node_info.add_other(unit, distance_weight);
                    }
                }
            }
        }
    }
};

WorldGraph::Pair get_nearest_node(const WorldGraph::Nodes& nodes, const Point& position);
WorldGraph::Node get_optimal_destination(const Context& context, const WorldGraph& graph, model::LaneType target_lane);

}

#endif
