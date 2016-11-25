#include "graph.hpp"

#include <algorithm>

namespace strategy {

Graph::Path Graph::get_shortest_path(Node src, Node dst) const {
    std::vector<double> lengths(size_, std::numeric_limits<double>::max());
    const auto less_priority = [&] (Node lhs, Node rhs) {
        return lengths[lhs] < lengths[rhs];
    };
    std::priority_queue<Node, std::deque<Node>, decltype(less_priority)> queue(less_priority);
    std::vector<bool> visited(size_, false);
    std::vector<Node> came_from(size_, size_);
    queue.push(src);
    while (!queue.empty()) {
        const auto node = queue.top();
        queue.pop();
        if (node == dst) {
            break;
        }
        visited[node] = true;
        for (Node other = 0; other < size_; ++other) {
            if (!visited[other]) {
                const auto weight = arcs_.get(node, other);
                if (weight != std::numeric_limits<double>::max()) {
                    lengths[other] = lengths[node] + weight;
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
    nodes.push_back(node);
    while (node != size_) {
        node = came_from[node];
        nodes.push_back(node);
    }
    std::reverse(nodes.begin(), nodes.end());
    return nodes;
}

}
