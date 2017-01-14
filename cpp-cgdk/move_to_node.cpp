#include "move_to_node.hpp"
#include "helpers.hpp"

namespace strategy {

MoveToNode::MoveToNode(std::vector<WorldGraph::Node> path)
        : path_(std::move(path)),
          path_node_(path_.begin()) {
}

bool MoveToNode::next(const Context& context) {
    if (path_node_ == path_.end()) {
        return false;
    }

    const auto to_next = path_node_->position.distance(get_position(context.self()));

    if (to_next > 0.5 * context.self().getVisionRange()) {
        return false;
    }

    if (path_node_ - path_.begin() > 0) {
        const auto prev = path_node_ - 1;
        const auto to_prev = prev->position.distance(get_position(context.self()));
        if (to_prev > to_next) {
            ++path_node_;
            return !at_end();
        }
    } else {
        ++path_node_;
        return !at_end();
    }

    return false;
}

} // namespace strategy
