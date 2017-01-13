#pragma once

#include "mode.hpp"
#include "move_to_node.hpp"

namespace strategy {

class MoveMode : public Mode {
public:
    MoveMode(const WorldGraph& graph);

    const std::vector<WorldGraph::Node>& path() const {
        return move_to_node_.path();
    }

    std::vector<WorldGraph::Node>::const_iterator path_node() const {
        return move_to_node_.path_node();
    }

    const std::pair<bool, WorldGraph::Node>& destination() const {
        return destination_;
    }

    model::LaneType target_lane() const {
        return target_lane_;
    }

    Result apply(const Context& context) override final;
    void reset() override final;

    const char* name() const override final {
        return "move";
    }

private:
    const WorldGraph& graph_;
    std::pair<bool, WorldGraph::Node> destination_;
    model::LaneType target_lane_ = model::_LANE_UNKNOWN_;
    Tick last_message_ = 0;
    MoveToNode move_to_node_;

    void handle_messages(const Context& context);
    void update_path(const Context& context);
    void next_path_node(const Context& context);
};

}
