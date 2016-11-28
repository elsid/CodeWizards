#ifndef STRATEGY_MOVE_MODE_HPP
#define STRATEGY_MOVE_MODE_HPP

#include "mode.hpp"
#include "world_graph.hpp"

namespace strategy {

class MoveMode : public Mode {
public:
    MoveMode(const WorldGraph& graph);

    const std::vector<WorldGraph::Node>& path() const {
        return path_;
    }

    const std::vector<WorldGraph::Node>::const_iterator& path_node() const {
        return path_node_;
    }

    const std::pair<bool, WorldGraph::Node>& destination() const {
        return destination_;
    }

    model::LaneType target_lane() const {
        return target_lane_;
    }

    Result apply(const Context& context) override final;

private:
    const WorldGraph& graph_;
    std::pair<bool, WorldGraph::Node> destination_;
    model::LaneType target_lane_ = model::_LANE_UNKNOWN_;
    Tick last_message_ = 0;
    std::vector<WorldGraph::Node> path_;
    std::vector<WorldGraph::Node>::const_iterator path_node_;

    void handle_messages(const Context& context);
    void update_path(const Context& context);
    void next_path_node(const Context& context);
};

}

#endif
