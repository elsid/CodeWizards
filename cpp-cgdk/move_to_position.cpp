#include "move_to_position.hpp"
#include "action.hpp"

namespace strategy {

MoveToPosition::MoveToPosition(const Context& context, const Point& destination, const Target& target)
        : destination_(destination), target_(target) {
    calculate_movements(context);
}

void MoveToPosition::next(const Context& context) {
    bool is_error = true;

    if (movement_ != movements_.end() && state_ != states_.end()) {
        const auto error = state_->position().distance(get_position(context.self())) - context.game().getWizardForwardSpeed();
        if (error <= 0) {
            ++movement_;
            ++state_;
            is_error = false;
        }
    }

    if (is_error || movement_ == movements_.end() || state_ == states_.end()) {
        calculate_movements(context);
    }
}

void MoveToPosition::calculate_movements(const Context& context) {
#ifdef ELSID_STRATEGY_DEBUG
    ticks_states_.clear();
    steps_states_.clear();
#endif
    const auto bounds = make_unit_bounds(context.self(), context.game());
    path_ = GetOptimalPath()
            .step_size(bounds.max_speed(0) + 1)
            .max_ticks(OPTIMAL_PATH_MAX_TICKS)
            .max_iterations(OPTIMAL_PATH_MAX_ITERATIONS)
#ifdef ELSID_STRATEGY_DEBUG
            .ticks_states(&ticks_states_)
            .steps_states(&steps_states_)
#endif
            (context, destination_);

    if (const auto unit = target_.circular_unit(context.cache())) {
        std::tie(states_, movements_) = get_optimal_movement(context, path_, {true, get_optimal_target_position(*unit)});
    } else {
        std::tie(states_, movements_) = get_optimal_movement(context, path_, {false, Point()});
    }

    state_ = states_.begin();
    movement_ = movements_.begin();
}

} // namespace strategy
