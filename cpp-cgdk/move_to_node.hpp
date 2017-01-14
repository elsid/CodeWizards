#pragma once

#include "world_graph.hpp"
#include "context.hpp"

namespace strategy {

class MoveToNode {
public:
    MoveToNode(std::vector<WorldGraph::Node> path);

    bool next(const Context& context);

    bool at_end() const {
        return path_node_ == path_.end();
    }

    const std::vector<WorldGraph::Node>& path() const {
        return path_;
    }

    std::vector<WorldGraph::Node>::const_iterator path_node() const {
        return path_node_;
    }

private:
    std::vector<WorldGraph::Node> path_;
    std::vector<WorldGraph::Node>::const_iterator path_node_;
};

} // namespace strategy
