#include "optimal_path.hpp"
#include "optimal_position.hpp"
#include "line.hpp"

#include <set>
#include <map>
#include <queue>
#include <unordered_map>
#include <unordered_set>

#ifdef ELSID_STRATEGY_DEBUG

#include "debug/output.hpp"

#endif

namespace strategy {

Circle make_circle(const model::CircularUnit* unit) {
    return Circle(get_position(*unit), unit->getRadius());
}

class TickState {
public:
    using Occupier = std::pair<bool, Circle>;

    TickState(std::vector<std::pair<Circle, Point>> dynamic_barriers,
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

    const std::vector<std::pair<Circle, Point>>& dynamic_barriers() const {
        return dynamic_barriers_;
    }

private:
    std::vector<std::pair<Circle, Point>> dynamic_barriers_;
    Occupier occupier_;
    double max_distance_error_;
};

class StepState {
public:
    StepState(double penalty, double tick, PointInt position)
        : priority_(penalty), tick_(tick), position_(position) {}

    double priority() const {
        return priority_;
    }

    double tick() const {
        return tick_;
    }

    const PointInt& position() const {
        return position_;
    }

private:
    double priority_;
    double distance_;
    double tick_;
    PointInt position_;
};

struct PositionState {
    double path_length;
    double priority;
    double sum_turn;
};

bool operator <(const StepState& lhs, const StepState& rhs) {
    return lhs.priority() < rhs.priority();
}

bool has_intersection_with_borders(const Circle& circle, double map_size) {
    const double delta = circle.radius() * 0.1;
    return circle.position().x() - circle.radius() <= delta
            || circle.position().y() - circle.radius() <= delta
            || circle.position().x() + circle.radius() - map_size >= delta
            || circle.position().y() + circle.radius() - map_size >= delta;
}

bool has_intersection_with_barriers(const Circle& barrier, const std::vector<Circle>& barriers) {
    return barriers.end() != std::find_if(barriers.begin(), barriers.end(),
        [&] (const Circle& v) { return v.has_intersection(barrier); });
}

bool has_intersection_with_barriers(const Circle& barrier, const Point& final_position, const std::vector<Circle>& barriers) {
    return barriers.end() != std::find_if(barriers.begin(), barriers.end(),
        [&] (const Circle& v) { return v.has_intersection(barrier, final_position); });
}

bool has_intersection_with_barriers(const Circle& barrier, const Point& final_position,
                                    const std::vector<std::pair<Circle, Point>>& barriers) {
    return barriers.end() != std::find_if(barriers.begin(), barriers.end(),
        [&] (const auto& v) { return v.first.has_intersection(v.second, barrier, final_position); });
}

Path reconstruct_path(PointInt position, const std::map<PointInt, PointInt>& came_from, const Point& shift,
                      const PointInt& target, const Point& target_shift) {
    Path result;
    std::set<PointInt> visited;
    result.reserve(came_from.size());
    while (true) {
        if (!visited.insert(position).second) {
            break;
        }
        result.push_back(position.to_double() + (position == target ? target_shift : shift));
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

Path get_optimal_path(const Context& context, const Point& target, int step_size, Tick max_ticks, std::size_t max_iterations) {
    const auto initial_position = get_position(context.self());
    const auto global_shift = initial_position.to_int().to_double() - initial_position;
    const auto initial_position_int = (initial_position - global_shift).to_int();

    const auto shifted = [&] (const PointInt& point) {
        return point.to_double() + global_shift;
    };

    const IsInMyRange is_in_my_range {context, 2 * target.distance(initial_position)};

    const auto initial_filter = [&] (const auto& units) {
        return filter_units(units, [&] (const auto& unit) { return !is_me(unit) && is_in_my_range(unit); });
    };

    const auto buildings = initial_filter(context.world().getBuildings());
    const auto minions = initial_filter(context.world().getMinions());
    const auto wizards = initial_filter(context.world().getWizards());
    const auto trees = initial_filter(context.world().getTrees());

    std::vector<Circle> static_barriers;
    static_barriers.reserve(buildings.size() + trees.size());
    std::transform(buildings.begin(), buildings.end(), std::back_inserter(static_barriers), make_circle);
    std::transform(trees.begin(), trees.end(), std::back_inserter(static_barriers), make_circle);

    const auto make_tick_state = [&] (double prev_tick, double tick) {
        const auto make_circle = [&] (auto unit) {
            return std::make_pair(Circle(get_position(*unit) + get_speed(*unit) * prev_tick, unit->getRadius()),
                                  get_position(*unit) + get_speed(*unit) * tick);
        };

        std::vector<std::pair<Circle, Point>> dynamic_barriers;
        dynamic_barriers.reserve(minions.size() + wizards.size());
        std::transform(minions.begin(), minions.end(), std::back_inserter(dynamic_barriers), make_circle);
        std::transform(wizards.begin(), wizards.end(), std::back_inserter(dynamic_barriers), make_circle);

        const Circle barrier(get_position(context.self()), context.self().getRadius());

        TickState::Occupier occupier;
        const auto static_occupier = std::find_if(static_barriers.begin(), static_barriers.end(),
            [&] (const auto& v) { return v.position().distance(target) <= v.radius() + barrier.radius(); });
        if (static_occupier != static_barriers.end()) {
            occupier = {true, *static_occupier};
        } else {
            const auto dynamic_occupier = std::find_if(dynamic_barriers.begin(), dynamic_barriers.end(),
                [&] (const auto& v) { return v.second.distance(target) <= v.first.radius() + barrier.radius(); });
            if (dynamic_occupier != dynamic_barriers.end()) {
                occupier = {true, dynamic_occupier->first};
            }
        }

        const auto max_distance_error = occupier.first ? occupier.second.radius() + barrier.radius() + 1 : 1;

        return TickState(std::move(dynamic_barriers), occupier, max_distance_error);
    };

    std::array<PointInt, 8> shifts = {{
        PointInt(step_size, 0),
        PointInt(step_size, step_size),
        PointInt(0, step_size),
        PointInt(-step_size, 0),
        PointInt(-step_size, -step_size),
        PointInt(0, -step_size),
        PointInt(step_size, -step_size),
        PointInt(-step_size, step_size),
    }};

    const auto max_range = target.distance(initial_position) + step_size;

    std::unordered_map<double, TickState> ticks_states({
        {0, make_tick_state(0, 0)},
    });

    const auto speed = (context.game().getWizardForwardSpeed() + context.game().getWizardBackwardSpeed()
                        + 2 * context.game().getWizardStrafeSpeed()) / 4;

    const auto has_intersection = [&] (const StepState& step_state, const TickState& tick_state, const PointInt& position) {
        const Circle barrier(shifted(step_state.position()), context.self().getRadius());
        return (shifted(step_state.position()).distance(initial_position) > max_range
                && shifted(step_state.position()).distance(target) > max_range)
                || has_intersection_with_borders(barrier, context.game().getMapSize())
                || has_intersection_with_barriers(barrier, shifted(position), static_barriers)
                || has_intersection_with_barriers(barrier, shifted(position), tick_state.dynamic_barriers());
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
        return -result;
    };

    const auto target_int = target.to_int();
    std::set<PointInt> closed;
    std::set<PointInt> opened({initial_position_int});
    std::map<PointInt, PointInt> came_from;
    std::map<PointInt, PositionState> positions;
    std::priority_queue<StepState, std::deque<StepState>> queue;
    bool found_path = false;
    PointInt final_position;
    PointInt optimal_position = initial_position_int;
    std::size_t iterations = 0;
    std::size_t current_max_iterations = max_iterations;
    const auto time_limit = context.time_limit();
    Duration max_duration(0);
    const auto distance = initial_position.distance(target);
    const auto distance_to_units_penalty = get_distance_to_units_penalty(Line(initial_position, initial_position));
    double max_priority = distance_to_units_penalty - 1.01 * distance;

    positions[initial_position_int] = {0, max_priority, 0};

    queue.push(StepState(max_priority, 0, initial_position_int));

    while (!queue.empty()) {
        context.check_timeout(__PRETTY_FUNCTION__, __FILE__, __LINE__);

        const auto iteration_start = Clock::now();

        const StepState step_state = queue.top();
        queue.pop();

        if (max_priority < step_state.priority()) {
            max_priority = step_state.priority();
            optimal_position = step_state.position();
        }

        const TickState& tick_state = ticks_states.at(step_state.tick());

        if (shifted(step_state.position()).distance(target) <= tick_state.max_distance_error()) {
            if (!tick_state.occupier().first && target != shifted(step_state.position())
                    && !came_from.count(target_int)) {
                const auto it = came_from.find(step_state.position());
                if (it != came_from.end()) {
                    came_from[target_int] = it->second;
                    final_position = target_int;
                    break;
                }
            }
            final_position = step_state.position();
            found_path = true;
            break;
        }

        if (++iterations >= current_max_iterations) {
            break;
        }

        if (step_state.tick() > max_ticks) {
            continue;
        }

        opened.erase(step_state.position());
        closed.insert(step_state.position());

        const auto position_state = positions.at(step_state.position());
        const auto prev_position = came_from.find(step_state.position());
        double rotation = 0;

        if (prev_position != came_from.end()) {
            rotation = (step_state.position() - prev_position->second).absolute_rotation();
        }

        for (std::size_t i = 0; i < shifts.size() + 1; ++i) {
            context.check_timeout(__PRETTY_FUNCTION__, __FILE__, __LINE__);

            const auto shift = i == 0 ? target_int - step_state.position() : shifts[i - 1];
            const auto position = step_state.position() + shift;
            if (i != 0 && closed.count(position)) {
                continue;
            }
            const auto path_length = shift.norm();
            const auto tick = step_state.tick() + path_length / speed;
            auto tick_state = ticks_states.find(tick);
            if (tick_state == ticks_states.end()) {
                tick_state = ticks_states.insert({tick, make_tick_state(step_state.tick(), tick)}).first;
            }
            if (has_intersection(step_state, tick_state->second, position)) {
                continue;
            }
            const auto distance_to_units_penalty = get_distance_to_units_penalty(Line(shifted(step_state.position()),
                                                                                      shifted(position)));
            const auto distance = target.distance(shifted(position));
            const auto sum_length = position_state.path_length + path_length;
            const auto turn = prev_position == came_from.end()
                    ? 0 : std::abs(rotation - (position - shift).absolute_rotation());
            const auto sum_turn = position_state.sum_turn + turn;
            const auto priority = distance_to_units_penalty - sum_length - 1.01 * distance - sum_turn;
            if (opened.insert(position).second) {
                queue.push(StepState(priority, tick, position));
            } else if (positions.at(position).priority > priority) {
                continue;
            }
            came_from[position] = step_state.position();
            positions[position] = {path_length, priority, sum_turn};
        }

        if (max_iterations != std::numeric_limits<std::size_t>::max() && time_limit != Duration::max()) {
            const auto iteration_finish = Clock::now();
            max_duration = std::max(max_duration, Duration(iteration_finish - iteration_start));
            if (iterations > 3) {
                current_max_iterations = std::min(current_max_iterations, std::size_t(std::floor(time_limit.count() / Duration(max_duration).count() * 0.5)));
            }
        }
    }

    if (!found_path) {
        final_position = optimal_position;
    }

    return reconstruct_path(final_position, came_from, global_shift, target_int, target - target.to_int().to_double());
}

}
