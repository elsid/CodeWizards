#include "optimal_movement.hpp"
#include "optimal_target.hpp"
#include "optimal_position.hpp"
#include "optimal_path.hpp"

#ifdef ELSID_STRATEGY_DEBUG

#include "debug/output.hpp"

#include <iostream>

#endif

namespace strategy {

Movement get_next_movement(const Point& target, const MovementState& state, const OptPoint& look_target, const Bounds& bounds) {
    double speed = 0;
    double strafe_speed = 0;
    double angle_to = 0;

    const auto direction = target - state.position();
    const auto norm = direction.norm();

    if (norm != 0) {
        angle_to = normalize_angle(direction.absolute_rotation() - state.angle());
        speed = math::cos(angle_to) * (std::abs(angle_to) <= M_PI_2 ? bounds.max_speed(state.tick()) : -bounds.min_speed(state.tick()));
        strafe_speed = bounds.max_strafe_speed(state.tick()) * math::sin(angle_to);
        const auto speed_factor = std::min(1.0, norm / std::hypot(speed, strafe_speed));
        speed *= speed_factor;
        strafe_speed *= speed_factor;
    }

    const auto turn = bounds.limit_turn(
                (look_target.first
                 ? normalize_angle((look_target.second - state.position()).absolute_rotation() - state.angle())
                 : angle_to), state.tick());

    return Movement(speed, strafe_speed, turn);
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
    MovementsStates states({MovementState(0, get_position(context.self()), context.self().getAngle())});
    Movements movements;

    if (path.empty()) {
        return {states, movements};
    }

    states.reserve(path.size());
    movements.reserve(path.size() - 1);

    const Bounds bounds {context};

    Point prev_path_position = path.front();
    for (const auto& path_position : path) {
        prev_path_position = path_position;
        while (path_position.distance(states.back().position()) > bounds.max_speed(std::floor(states.back().tick()))) {
            const auto next = get_next_state(path_position, states.back(), look_target, bounds);
            states.push_back(next.first);
            movements.push_back(next.second);
        }
    }

    while (look_target.first && normalize_angle((look_target.second - states.back().position()).absolute_rotation() - states.back().angle())
           > bounds.max_turn(std::floor(states.back().tick())) * 0.1) {
        const auto next = get_next_state(path.back(), states.back(), look_target, bounds);
        states.push_back(next.first);
        movements.push_back(next.second);
    }

    if (path.back() != states.back().position()) {
        const auto next = get_next_state(path.back(), states.back(), look_target, bounds);
        states.push_back(next.first);
        movements.push_back(next.second);
    }

    return {states, movements};
}

}
