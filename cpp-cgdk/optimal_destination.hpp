#ifndef STRATEGY_OPTIMAL_DESTINATION_HPP
#define STRATEGY_OPTIMAL_DESTINATION_HPP

#include "world_graph.hpp"
#include "context.hpp"
#include "optimal_target.hpp"
#include "optimal_position.hpp"

namespace strategy {

class GetNodePenalty {
public:
    static constexpr const double PARTIES_PENALTY_WEIGHT = 1.0;
    static constexpr const double PATH_LENGTH_PENALTY_WEIGHT = 0.5;
    static constexpr const double DISTANCE_TO_FRIEND_BASE_PENALTY_WEIGHT = 0.25;
    static constexpr const double DISTANCE_TO_ENEMY_BASE_PENALTY_WEIGHT = 0.25;
    static constexpr const double MAX_PENALTY = PARTIES_PENALTY_WEIGHT
            + PATH_LENGTH_PENALTY_WEIGHT
            + DISTANCE_TO_FRIEND_BASE_PENALTY_WEIGHT
            + DISTANCE_TO_ENEMY_BASE_PENALTY_WEIGHT;

    GetNodePenalty(const Context& context, const WorldGraph& graph, model::LaneType target_lane);

    double operator ()(WorldGraph::Node node) const;

private:
    struct NodeInfo {
        std::size_t enemies_count = 0;
        std::size_t friends_count = 0;
        WorldGraph::Path path;
        double distance_to_friend_base = 0;
        double distance_to_enemy_base = 0;
    };

    const Context& context_;
    const WorldGraph& graph_;
    model::LaneType target_lane_;
    std::vector<NodeInfo> nodes_info_;
    WorldGraph::Node self_nearest_node_;
    double max_path_length_ = 0;
    double max_distance_to_friend_base_ = 0;
    double max_distance_to_enemy_base_ = 0;
    std::size_t enemies_and_friends_count_ = 0;

    template <class Unit>
    void fill_nodes_info() {
        for (const auto& v : get_units<Unit>(context_.cache())) {
            const auto& unit = v.second.value();
            if (unit.getFaction() == model::FACTION_ACADEMY || unit.getFaction() == model::FACTION_RENEGADES) {
                const auto nearest_node = get_nearest_node(graph_.nodes(), get_position(unit)).first;
                auto& node_info = nodes_info_[nearest_node];
                if (unit.getFaction() == context_.self().getFaction()) {
                    ++node_info.friends_count;
                } else {
                    ++node_info.enemies_count;
                }
                ++enemies_and_friends_count_;
            }
        }
    }
};

WorldGraph::Pair get_nearest_node(const WorldGraph::Nodes& nodes, const Point& position);
WorldGraph::Node get_optimal_destination(const Context& context, const WorldGraph& graph, model::LaneType target_lane);

}

#endif
