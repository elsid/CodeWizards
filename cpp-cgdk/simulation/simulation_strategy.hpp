#pragma once

#include <abstract_strategy.hpp>
#include <action.hpp>
#include <move_to_node.hpp>
#include <move_to_position.hpp>

namespace strategy {
namespace simulation {

class SimulationStrategy : public AbstractStrategy {
public:
    SimulationStrategy(const Context& context);

    void apply(Context& context) override final;

    void move_to_node(const Context &context, WorldGraph::NodeId destination_node_id);
    void move_to_position(const Context &context, const Point& destination, const Target& target);
    void apply_action(const Action& action);

    bool is_at_node() const;

private:
    const WorldGraph graph_;
    MoveToNode move_to_node_;
    MoveToPosition move_to_position_;
    Action action_;
};

} // namespace simulation
} // namespace strategy
