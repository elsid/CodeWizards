#pragma once

#include "world_graph.hpp"
#include "context.hpp"
#include "optimal_target.hpp"

namespace strategy {

WorldGraph::Node get_optimal_destination(const Context& context, const WorldGraph& graph, model::LaneType target_lane);

}
