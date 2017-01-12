#include "world_graph.hpp"

#include <algorithm>

#ifdef ELSID_STRATEGY_DEBUG

#include "debug/output.hpp"

#include <iostream>

#endif

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
        add_node(shift, 0);
        lanes_nodes_[model::LANE_BOTTOM].insert(add_node(shift, tiles - 1));
        add_node(tiles - 1, shift);
        lanes_nodes_[model::LANE_MIDDLE].insert(add_node(shift, tiles - 1 - shift));
    }

    for (std::size_t shift = 2; shift < tiles - 2; ++shift) {
        lanes_nodes_[model::LANE_TOP].insert(get_node(shift, 0));
        lanes_nodes_[model::LANE_BOTTOM].insert(get_node(tiles - 1, shift));
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
    lanes_nodes_[model::LANE_MIDDLE].insert(get_node(tiles / 2 - 1, tiles / 2 - 1));

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

    for (std::size_t shift = 0; shift < tiles - 1; ++shift) {
        add_edge(0, tiles - 1 - shift, 0, tiles - 2 - shift);
        add_edge(shift, tiles - 1, shift + 1, tiles - 1);
        add_edge(tiles - 1 - shift, tiles - 1 - shift, tiles - 2 - shift, tiles - 2 - shift);
        add_edge(tiles - 1, shift, tiles - 1, shift + 1);
        add_edge(tiles - 1 - shift, 0, tiles - 2 - shift, 0);
    }

    add_edge(1, 1, 1, 0);
    add_edge(1, 1, 0, 1);
    add_edge(0, 1, 1, 0);
    add_edge(1, 0, 0, 1);

    add_edge(tiles - 2, 1, tiles - 2, 0);
    add_edge(tiles - 2, 1, tiles - 1, 1);
    add_edge(tiles - 2, 1, tiles - 3, 0);
    add_edge(tiles - 2, 1, tiles - 1, 2);

    add_edge(tiles - 2, tiles - 2, tiles - 2, tiles - 1);
    add_edge(tiles - 2, tiles - 2, tiles - 1, tiles - 2);
    add_edge(tiles - 1, tiles - 2, tiles - 2, tiles - 1);
    add_edge(tiles - 2, tiles - 1, tiles - 1, tiles - 2);

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

std::string render(const WorldGraph& world_graph, double map_size) {
    static constexpr std::size_t width = 40;
    static constexpr std::size_t height = 10;
    std::array<char, width * height> canvas;

    std::fill(canvas.begin(), canvas.end(), ' ');

    const auto& nodes = world_graph.nodes();
    const auto min_x = std::min_element(nodes.begin(), nodes.end(),
        [&] (const auto& lhs, const auto& rhs) { return lhs.second.x() < rhs.second.x(); })->second.x();
    const auto min_y = std::min_element(nodes.begin(), nodes.end(),
        [&] (const auto& lhs, const auto& rhs) { return lhs.second.y() < rhs.second.y(); })->second.y();

    for (const auto& v : nodes) {
        const std::size_t x = std::max(0, int(std::round((v.second.x() - min_x) * width / map_size)));
        const std::size_t y = std::max(0, int(std::round((v.second.y() - min_y) * height / map_size)));
        auto label = std::to_string(v.first);
        const auto& lanes_nodes = world_graph.lanes_nodes();
        const auto lane = std::find_if(lanes_nodes.begin(), lanes_nodes.end(),
            [&] (const auto& lane) { return lane.second.count(v.first); });

        if (lane != lanes_nodes.end()) {
            switch (lane->first) {
                case model::LANE_TOP:
                    label.push_back('T');
                    break;
                case model::LANE_MIDDLE:
                    label.push_back('M');
                    break;
                case model::LANE_BOTTOM:
                    label.push_back('B');
                    break;
                default:
                    break;
            }
        }

        std::copy(label.begin(), label.end(), canvas.begin() + x + y * width);
    }

    std::ostringstream result;

    for (std::size_t y = 0; y < height; ++y) {
        std::copy(canvas.begin() + width * y, canvas.begin() + width * (y + 1), std::ostreambuf_iterator<char>(result));
        result << '\n';
    }

    return result.str();
}

}
