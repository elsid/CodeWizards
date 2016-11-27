#ifndef STRATEGY_OPTIMAL_DESTINATION
#define STRATEGY_OPTIMAL_DESTINATION

#include "world_graph.hpp"
#include "context.hpp"
#include "optimal_target.hpp"

namespace strategy {

WorldGraph::Pair get_nearest_node(const WorldGraph::Nodes& nodes, const Point& position);
WorldGraph::Node get_optimal_destination(const Context& context, const WorldGraph& graph, model::LaneType target_lane);

}

#endif
