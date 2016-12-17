#include "move_mode.hpp"
#include "optimal_destination.hpp"

#include <iostream>

namespace strategy {

MoveMode::MoveMode(const WorldGraph& graph)
        : graph_(graph),
          destination_(false, WorldGraph::Node()),
          path_node_(path_.end())  {
}

MoveMode::Result MoveMode::apply(const Context& context) {
    handle_messages(context);
    update_path(context);

    return path_node_ == path_.end() ? Result() : Result(Target(), graph_.nodes().at(*path_node_));
}

void MoveMode::reset() {
    destination_.first = false;
    path_.clear();
    path_node_ = path_.end();
}

void MoveMode::handle_messages(const Context& context) {
    if (!context.self().getMessages().empty()) {
        last_message_ = context.world().getTickIndex();
        if (context.self().getMessages().back().getLane() != model::_LANE_COUNT_) {
            target_lane_ = context.self().getMessages().back().getLane();
        }
    } else if (context.world().getTickIndex() - last_message_ > MESSAGE_TICKS) {
        target_lane_ = model::_LANE_UNKNOWN_;
    }
}

void MoveMode::update_path(const Context& context) {
    if (!destination_.first || path_node_ == path_.end() || next_path_node(context)) {
        if (calculate_path(context)) {
            next_path_node(context);
        }
    }
}

bool MoveMode::calculate_path(const Context& context) {
    const auto destination = get_optimal_destination(context, graph_, target_lane_, context.self());
    if (destination_.first && destination == destination_.second) {
        return false;
    }
    destination_ = {true, destination};
    const auto nearest_node = get_nearest_node(graph_.nodes(), get_position(context.self())).first;
    path_ = graph_.get_shortest_path(nearest_node, destination).nodes;
    if (path_.empty()) {
        path_.push_back(nearest_node);
    }
    path_node_ = path_.begin();
    return true;
}

bool MoveMode::next_path_node(const Context& context) {
    if (path_node_ == path_.end()) {
        return false;
    }

    const auto to_next = graph_.nodes().at(*path_node_).distance(get_position(context.self()));

    if (to_next > 0.5 * context.self().getVisionRange()) {
        return false;
    }

    if (path_node_ - path_.begin() > 0) {
        const auto prev = path_node_ - 1;
        const auto to_prev = graph_.nodes().at(*prev).distance(get_position(context.self()));
        if (to_prev > to_next) {
            ++path_node_;
            return true;
        }
    } else {
        ++path_node_;
        return true;
    }

    return false;
}

}
