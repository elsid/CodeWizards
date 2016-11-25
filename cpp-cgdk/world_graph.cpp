#include "world_graph.hpp"

#include <algorithm>

namespace strategy {

WorldGraph::WorldGraph(const model::Game& game) : graph_(52) {
    const auto map_size = game.getMapSize();
    const auto resolution = map_size / 10;
    const auto half = resolution / 2;
    const std::size_t tiles = map_size / resolution;
    std::vector<Node> nodes(tiles * tiles, std::numeric_limits<Node>::max());

    const auto get_tile_point = [&] (std::size_t x, std::size_t y) {
        return Point(half + resolution * x, half + resolution * y);
    };

    const auto add_node = [&] (std::size_t x, std::size_t y) {
        const Node node = nodes_.size();
        nodes[x + tiles * y] = node;
        return nodes_.emplace(node, get_tile_point(x, y)).first->first;
    };

    const auto get_node = [&] (std::size_t x, std::size_t y) {
        return nodes[x + tiles * y];
    };

    for (std::size_t shift = 1; shift < tiles - 1; ++shift) {
        lanes_nodes_[model::LANE_TOP].insert(add_node(0, shift));
        lanes_nodes_[model::LANE_TOP].insert(add_node(shift, 0));
        lanes_nodes_[model::LANE_BOTTOM].insert(add_node(shift, tiles - 1));
        lanes_nodes_[model::LANE_BOTTOM].insert(add_node(tiles - 1, shift));
        lanes_nodes_[model::LANE_MIDDLE].insert(add_node(shift, tiles - 1 - shift));
    }

    add_node(0, tiles - 1);
    add_node(tiles - 1, 0);

    for (std::size_t shift = 0; shift < tiles; ++shift) {
        add_node(shift, shift);
    }

    lanes_nodes_[model::LANE_TOP].insert(get_node(0, 0));
    lanes_nodes_[model::LANE_TOP].insert(get_node(1, 1));
    lanes_nodes_[model::LANE_BOTTOM].insert(get_node(tiles - 1, tiles - 1));
    lanes_nodes_[model::LANE_BOTTOM].insert(get_node(tiles - 2, tiles - 2));
    lanes_nodes_[model::LANE_MIDDLE].insert(get_node(tiles / 2, tiles / 2));
    lanes_nodes_[model::LANE_MIDDLE].insert(get_node(tiles / 2 + 1, tiles / 2 + 1));

    const auto add_arc = [&] (std::size_t src_x, std::size_t src_y, std::size_t dst_x, std::size_t dst_y) {
        graph_.arc(get_node(src_x, src_y), get_node(dst_x, dst_y),
                   get_tile_point(src_x, src_y).distance(get_tile_point(dst_x, dst_y)));
    };

    const auto add_edge = [&] (std::size_t src_x, std::size_t src_y, std::size_t dst_x, std::size_t dst_y) {
        add_arc(src_x, src_y, dst_x, dst_y);
        add_arc(dst_x, dst_y, src_x, src_y);
    };

    for (std::size_t shift = 1; shift < tiles - 2; ++shift) {
        add_edge(shift, tiles - 1 - shift, shift + 1, tiles - 2 - shift);
    }

    for (std::size_t shift = 1; shift < tiles - 2; ++shift) {
        add_edge(0, tiles - 1 - shift, 0, tiles - 2 - shift);
        add_edge(shift, tiles - 1, shift + 1, tiles - 1);
        add_edge(tiles - 1 - shift, tiles - 1 - shift, tiles - 2 - shift, tiles - 2 - shift);
        add_edge(tiles - 1, shift, tiles - 1, shift + 1);
        add_edge(tiles - 1 - shift, 0, tiles - 2 - shift, 0);
    }

    add_edge(1, 1, 1, 0);
    add_edge(1, 1, 0, 1);
    add_edge(1, 1, 2, 0);
    add_edge(1, 1, 0, 2);

    add_edge(tiles - 2, 1, tiles - 2, 0);
    add_edge(tiles - 2, 1, tiles - 1, 1);
    add_edge(tiles - 2, 1, tiles - 3, 0);
    add_edge(tiles - 2, 1, tiles - 1, 2);

    add_edge(tiles - 2, tiles - 2, tiles - 2, tiles - 1);
    add_edge(tiles - 2, tiles - 2, tiles - 1, tiles - 2);
    add_edge(tiles - 2, tiles - 2, tiles - 3, tiles - 1);
    add_edge(tiles - 2, tiles - 2, tiles - 1, tiles - 3);

    add_edge(1, tiles - 2, 0, tiles - 2);
    add_edge(1, tiles - 2, 1, tiles - 1);
    add_edge(1, tiles - 2, 0, tiles - 3);
    add_edge(1, tiles - 2, 2, tiles - 1);

    add_edge(tiles / 2 - 1, tiles / 2 - 1, tiles / 2, tiles / 2 - 1);
    add_edge(tiles / 2, tiles / 2 - 1, tiles / 2, tiles / 2);
    add_edge(tiles / 2, tiles / 2, tiles / 2 - 1, tiles / 2);
    add_edge(tiles / 2 - 1, tiles / 2, tiles / 2 - 1, tiles / 2 - 1);

    center_ = get_node(tiles / 2 - 1, tiles / 2);
    friend_base_ = get_node(0, tiles - 1);
    enemy_base_ = get_node(tiles - 1, 0);
    zone_size_ = 1.5 * resolution;
}

WorldGraph::Path WorldGraph::get_shortest_path(Node src, Node dst) const {
    return graph_.get_shortest_path(src, dst);
}

}
