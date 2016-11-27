#include "move_mode.hpp"
#include "optimal_destination.hpp"

#ifdef STRATEGY_DEBUG

#include "debug/output.hpp"

#include <iostream>

#endif

namespace strategy {

MoveMode::MoveMode(const WorldGraph& graph)
        : graph_(graph),
          destination_(false, WorldGraph::Node()),
          departure_(false, WorldGraph::Node()) {
}

MoveMode::Result MoveMode::apply(const Context& context) {
    handle_messages(context);
    update_path(context);
    next_path_node(context);

    return path_node_ == path_.end() ? Result() : Result(Target(), graph_.nodes().at(*path_node_));
}

void MoveMode::handle_messages(const Context& context) {
    if (!context.self().getMessages().empty()) {
        last_message_ = context.world().getTickIndex();
        target_lane_ = context.self().getMessages().back().getLane();
    } else if (context.world().getTickIndex() - last_message_ > MESSAGE_TICKS) {
        target_lane_ = model::_LANE_UNKNOWN_;
    }
}

void MoveMode::update_path(const Context& context) {
    if (destination_.first) {
        return;
    }
    const auto destination = get_optimal_destination(context, graph_, target_lane_);
    const auto nearest_node = get_nearest_node(graph_.nodes(), get_position(context.self())).first;
    if (departure_.first && nearest_node == departure_.second) {
        return;
    }
    destination_ = {true, destination};
    departure_ = {true, nearest_node};
    path_ = graph_.get_shortest_path(nearest_node, destination).nodes;
    path_node_ = path_.begin();
}

void MoveMode::next_path_node(const Context& context) {
    if (path_node_ == path_.end()) {
        return;
    }

    const auto required_distance = 0.9 * (
        path_.end() - path_node_ > 2
        ? graph_.nodes().at(*path_node_).distance(graph_.nodes().at(*(path_node_ + 1)))
        : graph_.zone_size()
    );

    const auto distance = graph_.nodes().at(*path_node_).distance(get_position(context.self()));

    if (distance > required_distance) {
        return;
    }

    ++path_node_;
}

}
