#pragma once

#include "target.hpp"
#include "world_graph.hpp"
#include "optimal_movement.hpp"

namespace strategy {

class MoveToPosition {
public:
    MoveToPosition(const Context& context, const Point& destination, const Target& target);

    void next(const Context& context);

    bool at_end() const {
        return movement_ == movements_.end();
    }

    const Path& path() const {
        return path_;
    }

    const MovementsStates& states() const {
        return states_;
    }

    Movements::const_iterator movement() const {
        return movement_;
    }

    const std::map<int, TickState>& ticks_states() const {
        return ticks_states_;
    }

    const std::vector<StepState>& steps_states() const {
        return steps_states_;
    }

private:
    Point destination_;
    Target target_;
    Path path_;
    MovementsStates states_;
    Movements movements_;
    MovementsStates::const_iterator state_;
    Movements::const_iterator movement_;
    std::map<int, TickState> ticks_states_;
    std::vector<StepState> steps_states_;

    void calculate_movements(const Context& context);
};

} // namespace strategy
