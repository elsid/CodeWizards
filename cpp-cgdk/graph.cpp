#include "graph.hpp"

#include <algorithm>

namespace strategy {

Graph::Path Graph::get_shortest_path(Node src, Node dst) const {
    std::vector<double> lengths(size_, std::numeric_limits<double>::max());
    lengths[src] = 0;
    const auto less_priority = [&] (Node lhs, Node rhs) {
        return lengths[lhs] > lengths[rhs];
    };
    std::priority_queue<Node, std::vector<Node>, decltype(less_priority)> queue(less_priority);
    std::vector<Node> came_from(size_, size_);
    queue.push(src);
    while (!queue.empty()) {
        const auto node = queue.top();
        queue.pop();
        for (Node other = 0; other < size_; ++other) {
            const auto weight = arcs_.get(node, other);
            if (weight != std::numeric_limits<double>::max()) {
                const auto length = lengths[node] + weight;
                if (length < lengths[other]) {
                    lengths[other] = length;
                    came_from[other] = node;
                    queue.push(other);
                }
            }
        }
    }
    return came_from[dst] == size_ ? Path {0, {}} : Path {lengths[dst], reconstruct_path(dst, came_from)};
}

std::vector<Graph::Node> Graph::reconstruct_path(Node node, const std::vector<Node>& came_from) const {
    std::vector<Node> nodes;
    nodes.reserve(came_from.size());
    while (node != size_) {
        nodes.push_back(node);
        node = came_from.at(node);
    }
    std::reverse(nodes.begin(), nodes.end());
    return nodes;
}

}
