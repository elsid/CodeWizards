#include "optimal_movement.hpp"
#include "optimal_target.hpp"
#include "optimal_position.hpp"
#include "optimal_path.hpp"

namespace strategy {

struct Bounds {
    const Context& context;

    double max_speed(double tick) const {
    }

    double min_speed(double tick) const {
    }

    double max_turn(double tick) const {
    }

    double min_turn(double tick) const {
    }

    double limit_turn(double value, double tick) const {
        return std::min(max_turn(tick), std::max(min_turn(tick), value));
    }
};

double normalize_angle(double value) {
    if (value > M_PI) {
        return value - std::round(value / (2.0 * M_PI)) * 2.0 * M_PI;
    }
    if (value < -M_PI) {
        return value + std::round(std::abs(value) / (2.0 * M_PI)) * 2.0 * M_PI;
    }
    return value;
}

Movement get_next_movement(const Point& target, const MovementState& state, const Bounds& bounds) {
    const auto direction = target - state.position;
    const auto norm = direction.norm();
    if (norm == 0) {
        return Movement(0, 0, 0);
    }
    const auto turn = normalize_angle(direction.absolute_rotation() - state.angle);
    const auto speed = std::abs(turn) <= M_PI_2
            ? bounds.max_speed(tick) * std::cos(turn)
            : -bounds.min_speed(tick) * std::cos(turn);
    const auto strafe_speed = bounds.max_speed(tick) * std::sin(turn);
    const auto speed_factor = std::min(1.0, norm / std::hypot(speed, strafe_speed));
    return Movement(speed * speed_factor, strafe_speed * speed_factor, bounds.limit_turn(turn, tick));
}

std::pair<MovementState, Movement> get_next_state(const Point& target, const MovementState& state, const Point& look_target, const Bounds& bounds) {
    const auto movement = get_next_movement(target, state, bounds);
}

std::pair<MovementsStates, Movements> get_optimal_movement(const Context& context, const Path& path, const OptPoint& look_target) {
    const Bounds bounds {context};
    MovementsStates states({MovementState(0, get_position(context.self), context.self.angle)});
    Movements movements;

    states.reserve(path.size());
    movements.reserve(path.size() - 1);

    for (const auto& path_position : path) {
        while (path_position.distance(states.back().position) > bounds.max_speed(states.back().tick)) {
            const auto next = get_next_state(path_position, states.back(), look_target, bounds);
            states.push_back(next.first);
            movements.push_back(next.second);
        }
    }

    return std::pair<MovementsStates, Movements>();
}

}
