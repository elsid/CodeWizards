#include "move_to_position.hpp"
#include "action.hpp"

namespace strategy {

MoveToPosition::MoveToPosition(const Context& context, const Point& destination, const Target& target)
        : destination_(destination), target_(target) {
    calculate_movements(context);
}

void MoveToPosition::next(const Context& context) {
    if (movement_ == movements_.end() || state_ == states_.end() || ++state_ == states_.end() || ++movement_ == movements_.end()) {
        calculate_movements(context);
        return;
    }

    const auto error = state_->position().distance(get_position(context.self()));

    if (error > 1e-3) {
        SLOG(context) << "calculate_movements reason: error > 0, where error=" << error << '\n';
        calculate_movements(context);
        return;
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
