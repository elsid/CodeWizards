#ifndef STRATEGY_WORLD_GRAPH_HPP
#define STRATEGY_WORLD_GRAPH_HPP

#include "graph.hpp"
#include "point.hpp"

#include "model/Game.h"
#include "model/LaneType.h"

#include <map>
#include <unordered_map>
#include <unordered_set>

namespace strategy {

class WorldGraph {
public:
    using Node = Graph::Node;
    using Nodes = std::map<Node, Point>;
    using LanesNodes = std::unordered_map<model::LaneType, std::unordered_set<Node>>;
    using Path = Graph::Path;
    using Pair = std::pair<Node, Point>;

    WorldGraph(const model::Game& game);

    const Nodes& nodes() const { return nodes_; }
    const Matrix& arcs() const { return graph_.arcs(); }
    const LanesNodes& lanes_nodes() const { return lanes_nodes_; }
    double zone_size() const { return zone_size_; }
    Node center() const { return center_; }
    Node friend_base() const { return friend_base_; }
    Node enemy_base() const { return enemy_base_; }

    Path get_shortest_path(Node src, Node dst) const;

private:
    Graph graph_;
    Nodes nodes_;
    LanesNodes lanes_nodes_;
    double zone_size_;
    Node center_;
    Node friend_base_;
    Node enemy_base_;
};

}

#endif
