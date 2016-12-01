#ifndef STRATEGY_OPTIMAL_DESTINATION_HPP
#define STRATEGY_OPTIMAL_DESTINATION_HPP

#include "world_graph.hpp"
#include "context.hpp"
#include "optimal_target.hpp"
#include "optimal_position.hpp"

namespace strategy {

class GetNodeScore {
public:
    static constexpr const double ENEMY_WIZARD_REDUCE_FACTOR = 4.0;
    static constexpr const double ENEMY_MINION_REDUCE_FACTOR = 2.0;
    static constexpr const double ENEMY_TOWER_REDUCE_FACTOR = 400.0;
    static constexpr const double ENEMY_BASE_REDUCE_FACTOR = 800.0;
    static constexpr const double FRIEND_WIZARD_REDUCE_FACTOR = 2.0;
    static constexpr const double FRIEND_MINION_REDUCE_FACTOR = 1.0;
    static constexpr const double PATH_LENGTH_REDUCE_FACTOR = 0.1;
    static constexpr const double FRIEND_TOWER_MULT_FACTOR = 1.0;
    static constexpr const double FRIEND_BASE_MULT_FACTOR = 2.0;
    static constexpr const double WIZARD_DAMAGE_PROBABILITY = 0.25;
    static constexpr const double WIZARD_ELIMINATION_PROBABILITY = 0.05;

    GetNodeScore(const Context& context, const WorldGraph& graph, model::LaneType target_lane);

    double operator ()(WorldGraph::Node node) const;

private:
    struct NodeInfo {
        std::size_t enemy_wizards_count = 0;
        std::size_t enemy_minions_count = 0;
        std::size_t enemy_towers_count = 0;
        std::size_t friend_wizards_count = 0;
        std::size_t friend_minions_count = 0;
        std::size_t friend_towers_count = 0;
        bool has_bonus = false;
        bool has_enemy_base = false;
        bool has_friend_base = false;
        WorldGraph::Path path;

        void add_other(const model::Unit&) {}

        void add_other(const model::Bonus&) {
            has_bonus = true;
        }

        void add_enemy(const model::Unit&) {}

        void add_enemy(const model::Wizard&) {
            ++enemy_wizards_count;
        }

        void add_enemy(const model::Minion&) {
            ++enemy_minions_count;
        }

        void add_enemy(const model::Building& unit) {
            if (unit.getType() == model::BUILDING_FACTION_BASE) {
                has_enemy_base = true;
            } else {
                ++enemy_towers_count;
            }
        }

        void add_friend(const model::Unit&) {}

        void add_friend(const model::Wizard&) {
            ++friend_wizards_count;
        }

        void add_friend(const model::Minion&) {
            ++friend_minions_count;
        }

        void add_friend(const model::Building& unit) {
            if (unit.getType() == model::BUILDING_FACTION_BASE) {
                has_friend_base = true;
            } else {
                ++friend_towers_count;
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
            const auto nearest_node = get_nearest_node(graph_.nodes(), get_position(unit)).first;
            auto& node_info = nodes_info_[nearest_node];
            if (unit.getFaction() == model::FACTION_ACADEMY || unit.getFaction() == model::FACTION_RENEGADES) {
                if (unit.getFaction() == context_.self().getFaction()) {
                    node_info.add_friend(unit);
                } else {
                    node_info.add_enemy(unit);
                }
            } else {
                node_info.add_other(unit);
            }
        }
    }
};

WorldGraph::Pair get_nearest_node(const WorldGraph::Nodes& nodes, const Point& position);
WorldGraph::Node get_optimal_destination(const Context& context, const WorldGraph& graph, model::LaneType target_lane);

}

#endif
