#include "move_mode.hpp"
#include "optimal_destination.hpp"

#include <iostream>
#include <iterator>

namespace model {

inline std::ostream& operator <<(std::ostream& stream, LaneType value) {
    switch (value) {
        case _LANE_UNKNOWN_:
            return stream << "model::_LANE_UNKNOWN_";
        case LANE_TOP:
            return stream << "model::LANE_TOP";
        case LANE_MIDDLE:
            return stream << "model::LANE_MIDDLE";
        case LANE_BOTTOM:
            return stream << "model::LANE_BOTTOM";
        case _LANE_COUNT_:
            return stream << "model::_LANE_COUNT_";
    }
    return stream;
}

}

namespace strategy {

std::ostream& operator <<(std::ostream& stream, const std::vector<WorldGraph::Node>& value) {
    stream << '{';
    std::transform(value.begin(), value.end(), std::ostream_iterator<WorldGraph::NodeId>(stream, ", "),
        [&] (const auto& node) { return node.id; });
    return stream << '}';
}

MoveMode::MoveMode(const WorldGraph& graph)
        : graph_(graph),
          destination_(false, WorldGraph::Node()),
          move_to_node_({})  {
}

MoveMode::Result MoveMode::apply(const Context& context) {
    if (!context.self().isMaster()) {
        handle_messages(context);
    }
    update_path(context);
    next_path_node(context);

    return move_to_node_.at_end() ? Result() : Result(Target(), move_to_node_.path_node()->position);
}

void MoveMode::reset() {
    destination_.first = false;
    move_to_node_ = MoveToNode({});
}

void MoveMode::handle_messages(const Context& context) {
    if (!context.self().getMessages().empty()) {
        last_message_ = context.world().getTickIndex();
        const auto lane = context.self().getMessages().back().getLane();
        if (lane != model::_LANE_COUNT_ && lane != target_lane_) {
            target_lane_ = lane;
            destination_.first = false;
            SLOG(context) << "change_target_lane " << target_lane_ << '\n';
        }
    } else if (context.world().getTickIndex() - last_message_ > MESSAGE_TICKS) {
        target_lane_ = model::_LANE_UNKNOWN_;
        destination_.first = false;
        SLOG(context) << "change_target_lane " << target_lane_ << '\n';
    }
}

void MoveMode::update_path(const Context& context) {
    if (destination_.first && !move_to_node_.at_end()) {
        return;
    }
    const auto destination = get_optimal_destination(context, graph_, target_lane_, context.self());
    if (destination_.first && destination_.second.id == destination.id) {
        return;
    }
    destination_ = {true, destination};
    const auto nearest_node = get_nearest_node(graph_.nodes(), get_position(context.self()));
    auto path = graph_.get_shortest_path(nearest_node.id, destination.id).nodes;
    if (path.empty()) {
        path.push_back(nearest_node);
    }
    move_to_node_ = MoveToNode(std::move(path));

    SLOG(context) << "move_to_node"
        << " source=" << nearest_node.id
        << ", destination=" << destination.id
        << ", path=" << move_to_node_.path()
        << '\n';
}

void MoveMode::next_path_node(const Context& context) {
    move_to_node_.next(context);
}

}
