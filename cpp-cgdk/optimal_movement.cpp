#include "optimal_movement.hpp"
#include "optimal_target.hpp"
#include "optimal_position.hpp"
#include "optimal_path.hpp"

#ifdef STRATEGY_DEBUG

#include "debug/output.hpp"

#endif

namespace strategy {

int get_hastened_remaining_ticks(const model::LivingUnit& unit) {
    const auto hastend = find_status(unit.getStatuses(), model::STATUS_HASTENED);
    return hastend == unit.getStatuses().end() ? 0 : hastend->getRemainingDurationTicks();
}

double normalize_angle(double value) {
    if (value > M_PI) {
        return value - std::round(value / (2.0 * M_PI)) * 2.0 * M_PI;
    }
    if (value < -M_PI) {
        return value + std::round(std::abs(value) / (2.0 * M_PI)) * 2.0 * M_PI;
    }
    return value;
}

Movement get_next_movement(const Point& target, const MovementState& state, const OptPoint& look_target, const Bounds& bounds) {
    const auto direction = target - state.position();
    const auto norm = direction.norm();
    if (norm == 0) {
        return Movement(0, 0, 0);
    }
    const auto angle_to = normalize_angle(direction.absolute_rotation() - state.angle());
    const auto speed = std::cos(angle_to)
            * (std::abs(angle_to) <= M_PI_2 ? bounds.max_speed(state.tick()) : -bounds.min_speed(state.tick()));
    const auto strafe_speed = bounds.max_strafe_speed(state.tick()) * std::sin(angle_to);
    const auto speed_factor = std::min(1.0, norm / std::hypot(speed, strafe_speed));
    const auto turn = look_target.first
            ? normalize_angle((look_target.second - state.position()).absolute_rotation() - state.angle())
            : angle_to;
    return Movement(speed * speed_factor, strafe_speed * speed_factor, bounds.limit_turn(turn, state.tick()));
}

Point get_shift(const MovementState& state, const Movement& movement) {
    const auto speed_direction = Point(1, 0).rotated(state.angle());
    const auto strafe_speed_direction = speed_direction.left_orthogonal();
    return speed_direction * movement.speed() + strafe_speed_direction * movement.strafe_speed();
}

std::pair<MovementState, Movement> get_next_state(const Point& target, const MovementState& state, const OptPoint& look_target, const Bounds& bounds) {
    const auto movement = get_next_movement(target, state, look_target, bounds);
    const auto shift = get_shift(state, movement);
    const auto position = state.position() + shift;
    const auto angle = normalize_angle(state.angle() + movement.turn());
    return {MovementState(state.tick() + 1, position, angle), movement};
}

std::pair<MovementsStates, Movements> get_optimal_movement(const Context& context, const Path& path, const OptPoint& look_target) {
    const Bounds bounds {context};
    MovementsStates states({MovementState(0, get_position(context.self()), context.self().getAngle())});
    Movements movements;

    states.reserve(path.size());
    movements.reserve(path.size() - 1);

    for (const auto& path_position : path) {
        while (path_position.distance(states.back().position()) > bounds.max_speed(states.back().tick())) {
            const auto next = get_next_state(path_position, states.back(), look_target, bounds);
            states.push_back(next.first);
            movements.push_back(next.second);
        }
    }

    if (!path.empty()) {
        const auto next = get_next_state(path.back(), states.back(), look_target, bounds);
        states.push_back(next.first);
        movements.push_back(next.second);
    }

    return {states, movements};
}

}
