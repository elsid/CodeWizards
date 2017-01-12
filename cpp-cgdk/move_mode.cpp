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
    std::copy(value.begin(), value.end(), std::ostream_iterator<WorldGraph::Node>(stream, ", "));
    return stream << '}';
}

MoveMode::MoveMode(const WorldGraph& graph)
        : graph_(graph),
          destination_(false, WorldGraph::Node()),
          path_node_(path_.end())  {
}

MoveMode::Result MoveMode::apply(const Context& context) {
    if (!context.self().isMaster()) {
        handle_messages(context);
    }
    update_path(context);
    next_path_node(context);

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
    if (destination_.first && path_node_ != path_.end()) {
        return;
    }
    const auto destination = get_optimal_destination(context, graph_, target_lane_, context.self());
    if (destination_.first && destination_.second == destination) {
        return;
    }
    destination_ = {true, destination};
    const auto nearest_node = get_nearest_node(graph_.nodes(), get_position(context.self())).first;
    path_ = graph_.get_shortest_path(nearest_node, destination).nodes;
    if (path_.empty()) {
        path_.push_back(nearest_node);
    }
    path_node_ = path_.begin();
    SLOG(context) << "move_to_node source=" << nearest_node << ", destination=" << destination << ", path=" << path_ << '\n';
}

void MoveMode::next_path_node(const Context& context) {
    if (path_node_ == path_.end()) {
        return;
    }

    const auto to_next = graph_.nodes().at(*path_node_).distance(get_position(context.self()));

    if (to_next > 0.5 * context.self().getVisionRange()) {
        return;
    }

    if (path_node_ - path_.begin() > 0) {
        const auto prev = path_node_ - 1;
        const auto to_prev = graph_.nodes().at(*prev).distance(get_position(context.self()));
        if (to_prev > to_next) {
            ++path_node_;
        }
    } else {
        ++path_node_;
    }
}

}
