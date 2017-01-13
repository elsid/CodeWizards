#pragma once

#include "graph.hpp"
#include "point.hpp"

#include "model/Game.h"
#include "model/LaneType.h"

#include <map>
#include <unordered_map>
#include <unordered_set>
#include <string>

namespace strategy {

class WorldGraph {
public:
    using NodeId = Graph::Node;

    struct Node {
        NodeId id;
        Point position;
        model::LaneType lane;
    };

    using Nodes = std::map<NodeId, Node>;
    using LanesNodes = std::unordered_map<model::LaneType, std::unordered_set<NodeId>>;

    struct Path {
        double length;
        std::vector<Node> nodes;
    };

    WorldGraph(const model::Game& game);

    const Nodes& nodes() const { return nodes_; }
    const Matrix& arcs() const { return graph_.arcs(); }
    const LanesNodes& lanes_nodes() const { return lanes_nodes_; }
    double zone_size() const { return zone_size_; }
    NodeId center() const { return center_; }
    NodeId friend_base() const { return friend_base_; }
    NodeId enemy_base() const { return enemy_base_; }

    Path get_shortest_path(NodeId src, NodeId dst) const;

private:
    Graph graph_;
    Nodes nodes_;
    LanesNodes lanes_nodes_;
    double zone_size_;
    NodeId center_;
    NodeId friend_base_;
    NodeId enemy_base_;
};

std::string render(const WorldGraph& world_graph, double map_size);

}
