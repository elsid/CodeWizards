#include "simulation_strategy.hpp"

#include <optimal_destination.hpp>

namespace strategy {
namespace simulation {

SimulationStrategy::SimulationStrategy(const Context& context)
        : graph_(context.game()),
          move_to_node_({}),
          move_to_position_(context, get_position(context.self()), Target()) {
}

void SimulationStrategy::apply(Context &context) {
    if (!move_to_node_.at_end()) {
        if (move_to_node_.next(context)) {
            move_to_position_ = MoveToPosition(context, move_to_node_.path_node()->position, Target());
        } else {
            move_to_position_ = MoveToPosition(context, get_position(context.self()), Target());
        }
    }

    if (!move_to_position_.at_end()) {
        context.move().setSpeed(move_to_position_.movement()->speed());
        context.move().setStrafeSpeed(move_to_position_.movement()->strafe_speed());
        context.move().setTurn(move_to_position_.movement()->turn());
    }

    context.move().setAction(action_.type());
    context.move().setCastAngle(action_.cast_angle());
    context.move().setMinCastDistance(action_.min_cast_distance());
    context.move().setMaxCastDistance(action_.max_cast_distance());
    context.move().setStatusTargetId(action_.status_target_id());

    action_ = Action();

    move_to_position_.next(context);
}

void SimulationStrategy::move_to_node(const Context &context, WorldGraph::NodeId destination_node_id) {
    const auto source = get_nearest_node(graph_.nodes(), get_position(context.self()));
    move_to_node_ = MoveToNode(graph_.get_shortest_path(source.id, destination_node_id).nodes);
}

void SimulationStrategy::move_to_position(const Context& context, const Point& destination, const Target& target) {
    move_to_position_ = MoveToPosition(context, destination, target);
}

void SimulationStrategy::apply_action(const Action& action) {
    action_ = action;
}

bool SimulationStrategy::is_at_node() const {
    return move_to_node_.at_end();
}

} // namespace simulation
} // namespace strategy
