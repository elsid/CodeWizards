#include "world_graph.hpp"

#include <algorithm>
#include <array>
#include <sstream>

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
    std::vector<NodeId> nodes(tiles * tiles, std::numeric_limits<NodeId>::max());

    const auto get_tile_point = [&] (std::size_t x, std::size_t y) {
        return Point(half + resolution * x, half + resolution * y);
    };

    const auto add_node = [&] (std::size_t x, std::size_t y, model::LaneType lane) {
        const NodeId id = nodes_.size();
        nodes[x + tiles * y] = id;

        if (lane != model::_LANE_UNKNOWN_ && lane != model::_LANE_COUNT_) {
            lanes_nodes_[lane].insert(id);
        }

        nodes_.emplace(id, Node {id, get_tile_point(x, y), lane});
    };

    const auto get_node_id = [&] (std::size_t x, std::size_t y) {
        return nodes[x + tiles * y];
    };

    const auto add_node_to_lane = [&] (NodeId id, model::LaneType lane) {
        lanes_nodes_[lane].insert(id);
        nodes_[id].lane = lane;
    };

    for (std::size_t shift = 1; shift < tiles - 1; ++shift) {
        add_node(0, shift, model::LANE_TOP);
        add_node(shift, 0, model::LANE_TOP);
        add_node(shift, tiles - 1, model::LANE_BOTTOM);
        add_node(tiles - 1, shift, model::LANE_BOTTOM);
        add_node(shift, tiles - 1 - shift, model::LANE_MIDDLE);
    }

    for (std::size_t shift = 2; shift < tiles - 2; ++shift) {
        add_node_to_lane(get_node_id(shift, 0), model::LANE_TOP);
        add_node_to_lane(get_node_id(tiles - 1, shift), model::LANE_BOTTOM);
    }

    add_node(0, tiles - 1, model::_LANE_UNKNOWN_);
    add_node(tiles - 1, 0, model::_LANE_UNKNOWN_);

    for (std::size_t shift = 0; shift < tiles; ++shift) {
        add_node(shift, shift, model::_LANE_UNKNOWN_);
    }

    add_node_to_lane(get_node_id(0, 0), model::LANE_TOP);
    add_node_to_lane(get_node_id(1, 1), model::LANE_TOP);
    add_node_to_lane(get_node_id(tiles - 1, tiles - 1), model::LANE_BOTTOM);
    add_node_to_lane(get_node_id(tiles - 2, tiles - 2), model::LANE_BOTTOM);
    add_node_to_lane(get_node_id(tiles / 2, tiles / 2), model::LANE_MIDDLE);
    add_node_to_lane(get_node_id(tiles / 2 - 1, tiles / 2 - 1), model::LANE_MIDDLE);

    const auto add_arc = [&] (std::size_t src_x, std::size_t src_y, std::size_t dst_x, std::size_t dst_y) {
        graph_.arc(get_node_id(src_x, src_y), get_node_id(dst_x, dst_y),
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

    center_ = get_node_id(tiles / 2 - 1, tiles / 2);
    friend_base_ = get_node_id(0, tiles - 1);
    enemy_base_ = get_node_id(tiles - 1, 0);
    zone_size_ = 1.5 * resolution;
}

WorldGraph::Path WorldGraph::get_shortest_path(NodeId src, NodeId dst) const {
    const auto path = graph_.get_shortest_path(src, dst);
    WorldGraph::Path result {path.length, {}};
    result.nodes.reserve(path.nodes.size());
    std::transform(path.nodes.begin(), path.nodes.end(), std::back_inserter(result.nodes), [&] (auto id) { return this->nodes().at(id); });
    return result;
}

std::string render(const WorldGraph& world_graph, double map_size) {
    static constexpr std::size_t width = 40;
    static constexpr std::size_t height = 10;
    std::array<char, width * height> canvas;

    std::fill(canvas.begin(), canvas.end(), ' ');

    const auto& nodes = world_graph.nodes();
    const auto min_x = std::min_element(nodes.begin(), nodes.end(),
        [&] (const auto& lhs, const auto& rhs) { return lhs.second.position.x() < rhs.second.position.x(); })->second.position.x();
    const auto min_y = std::min_element(nodes.begin(), nodes.end(),
        [&] (const auto& lhs, const auto& rhs) { return lhs.second.position.y() < rhs.second.position.y(); })->second.position.y();

    for (const auto& v : nodes) {
        const auto& node = v.second;
        const std::size_t x = std::max(0, int(std::round((node.position.x() - min_x) * width / map_size)));
        const std::size_t y = std::max(0, int(std::round((node.position.y() - min_y) * height / map_size)));
        auto label = std::to_string(v.first);

        switch (node.lane) {
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
