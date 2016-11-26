#include "optimal_path.hpp"
#include "optimal_position.hpp"
#include "line.hpp"

#include <set>
#include <map>
#include <queue>
#include <unordered_map>

namespace strategy {

Circle make_circle(const model::CircularUnit* unit) {
    return Circle(get_position(*unit), unit->getRadius());
}

class TickState {
public:
    using Occupier = std::pair<bool, Circle>;

    TickState(std::vector<Circle> dynamic_barriers,
              const Occupier& occupier, double max_distance_error)
            : dynamic_barriers_(std::move(dynamic_barriers)),
              occupier_(occupier),
              max_distance_error_(max_distance_error) {}

    double max_distance_error() const {
        return max_distance_error_;
    }

    const Occupier& occupier() const {
        return occupier_;
    }

    const std::vector<Circle>& dynamic_barriers() const {
        return dynamic_barriers_;
    }

private:
    std::vector<Circle> dynamic_barriers_;
    Occupier occupier_;
    double max_distance_error_;
};

class StepState {
public:
    StepState(double penalty, double distance, double tick, Point position)
        : penalty_(penalty), distance_(distance), tick_(tick), position_(position) {}

    double penalty() const {
        return penalty_;
    }

    double distance() const {
        return distance_;
    }

    double tick() const {
        return tick_;
    }

    const Point& position() const {
        return position_;
    }

private:
    double penalty_;
    double distance_;
    double tick_;
    Point position_;
};

bool operator <(const StepState& lhs, const StepState& rhs) {
    return lhs.penalty() < rhs.penalty();
}

bool has_intersection_with_borders(const Circle& circle, double map_size) {
    const double delta = circle.radius() * 0.1;
    return circle.position().x() - circle.radius() <= delta
            || circle.position().y() - circle.radius() <= delta
            || circle.position().x() + circle.radius() - map_size >= delta
            || circle.position().y() + circle.radius() - map_size >= delta;
}

bool has_intersection_with_barriers(const Circle& barrier, const Point& final_position, const std::vector<Circle>& barriers) {
    return barriers.end() != std::find_if(barriers.begin(), barriers.end(),
        [&] (const Circle& v) { return v.has_intersection_with_moving_circle(barrier, final_position); });
}

Path reconstruct_path(Point position, const std::map<Point, Point>& came_from) {
    Path result;
    result.reserve(came_from.size());
    result.push_back(position);
    while (true) {
        const auto prev = came_from.find(position);
        if (prev == came_from.end()) {
            break;
        }
        position = prev->second;
    }
    std::reverse(result.begin(), result.end());
    return result;
}

template <class T>
auto get_closest_unit(const std::vector<const T*>& units, const Line& path) {
    return std::min_element(units.begin(), units.end(),
         [&] (auto lhs, auto rhs) {
             return path.distance(get_position(*lhs)) < path.distance(get_position(*rhs));
         });
}

template <class T>
double get_distance_to_closest_unit(const std::vector<const T*>& units, const Line& path) {
    return path.distance(get_position(**get_closest_unit(units, path)));
}

Path get_optimal_path(const Context& context, const Point& target, double step_size) {
    const auto initial_position = get_position(context.self);
    const IsInMyRange is_in_my_range {context, 2 * target.distance(initial_position)};

    const auto initial_filter = [&] (const auto& units) {
        return filter_units(units, [&] (const auto& unit) { return !is_me(unit) && is_in_my_range(unit); });
    };

    const auto buildings = initial_filter(context.world.getBuildings());
    const auto minions = initial_filter(context.world.getMinions());
    const auto wizards = initial_filter(context.world.getWizards());
    const auto trees = initial_filter(context.world.getTrees());

    std::vector<Circle> static_barriers;
    static_barriers.reserve(buildings.size() + trees.size());
    std::transform(buildings.begin(), buildings.end(), std::back_inserter(static_barriers), make_circle);
    std::transform(trees.begin(), trees.end(), std::back_inserter(static_barriers), make_circle);

    const auto make_tick_state = [&] (double tick) {
        const auto make_circle = [&] (auto unit) {
            return Circle(get_position(*unit) + get_speed(*unit) * tick, unit->getRadius());
        };

        std::vector<Circle> dynamic_barriers;
        dynamic_barriers.reserve(minions.size() + wizards.size());
        std::transform(minions.begin(), minions.end(), std::back_inserter(dynamic_barriers), make_circle);
        std::transform(wizards.begin(), wizards.end(), std::back_inserter(dynamic_barriers), make_circle);

        const Circle barrier(get_position(context.self), context.self.getRadius());

        const auto find_occupier = [&] (const auto& barriers) {
            return std::find_if(barriers.begin(), barriers.end(),
                [&] (const auto& v) { return v.position().distance(target) < v.radius() + barrier.radius(); });
        };

        TickState::Occupier occupier;
        const auto static_occupier = find_occupier(static_barriers);
        if (static_occupier != static_barriers.end()) {
            occupier = {true, *static_occupier};
        } else {
            const auto dynamic_occupier = find_occupier(dynamic_barriers);
            if (dynamic_occupier != dynamic_barriers.end()) {
                occupier = {true, *dynamic_occupier};
            }
        }

        const auto max_distance_error = occupier.first ? occupier.second.radius() + barrier.radius() + step_size : step_size;

        return TickState(std::move(dynamic_barriers), occupier, max_distance_error);
    };

    std::vector<Point> shifts = {
        Point(-step_size, -step_size),
        Point(-step_size, 0),
        Point(-step_size, step_size),
        Point(0, -step_size),
        Point(0, step_size),
        Point(step_size, -step_size),
        Point(step_size, 0),
        Point(step_size, step_size),
    };

    const auto max_range = target.distance(initial_position) / 2 + step_size;

    std::unordered_map<double, TickState> ticks_states({
        {0, make_tick_state(0)},
    });

    const auto speed = (context.game.getWizardForwardSpeed() + context.game.getWizardBackwardSpeed()
                        + 2 * context.game.getWizardStrafeSpeed()) / 4;

    const auto has_intersection = [&] (const StepState& step_state, const TickState& tick_state, const Point& position) {
        const Circle barrier(step_state.position(), context.self.getRadius());
        return (step_state.position().distance(initial_position) > max_range
                && step_state.position().distance(target) > max_range)
                || has_intersection_with_borders(barrier, context.game.getMapSize())
                || has_intersection_with_barriers(barrier, position, static_barriers)
                || has_intersection_with_barriers(barrier, position, tick_state.dynamic_barriers());
    };

    const auto get_distance_to_units_penalty = [&] (const Line& path) {
        double result = 0;
        if (!buildings.empty()) {
            result = std::min(result, get_distance_to_closest_unit(buildings, path));
        }
        if (!minions.empty()) {
            result = std::min(result, get_distance_to_closest_unit(minions, path));
        }
        if (!wizards.empty()) {
            result = std::min(result, get_distance_to_closest_unit(wizards, path));
        }
        if (!trees.empty()) {
            result = std::min(result, get_distance_to_closest_unit(trees, path));
        }
        return result;
    };

    std::set<Point> closed;
    std::set<Point> opened({initial_position});
    std::map<Point, Point> came_from;
    std::map<Point, double> penalties;
    std::priority_queue<StepState, std::deque<StepState>> queue;
    Point final_position;

    queue.push(StepState(0, target.distance(initial_position), 0, initial_position));

    while (!queue.empty()) {
        const StepState step_state = queue.top();
        queue.pop();

        const TickState& tick_state = ticks_states.at(step_state.tick());

        if (step_state.distance() <= tick_state.max_distance_error()) {
            if (!tick_state.occupier().first && target != step_state.position()
                    && !came_from.count(target) && !came_from.count(step_state.position())) {
                came_from[target] = came_from[step_state.position()];
                final_position = target;
            } else {
                final_position = step_state.position();
            }
            break;
        }

        opened.erase(step_state.position());
        closed.insert(step_state.position());

        for (std::size_t i = 0; i < shifts.size() + 1; ++i) {
            const auto shift = i == 0 ? target - step_state.position() : shifts[i - 1];
            const auto position = step_state.position() + shift;
            if (i != 0 && closed.count(position)) {
                continue;
            }
            const auto length = shift.norm();
            const auto tick = step_state.tick() + length / speed;
            auto tick_state = ticks_states.find(tick);
            if (tick_state == ticks_states.end()) {
                tick_state = ticks_states.insert({tick, make_tick_state(tick)}).first;
            }
            if (has_intersection(step_state, tick_state->second, position)) {
                continue;
            }
            const auto penalty = penalties[position] + length
                    + get_distance_to_units_penalty(Line(step_state.position(), position));
            if (!opened.count(position)) {
                queue.push(StepState(penalty, target.distance(position), tick, position));
                opened.insert(step_state.position());
            } else if (penalty > penalties[step_state.position()]) {
                continue;
            }
            came_from[position] = step_state.position();
            penalties[position] = penalty;
        }
    }

    return reconstruct_path(final_position, came_from);
}

}
