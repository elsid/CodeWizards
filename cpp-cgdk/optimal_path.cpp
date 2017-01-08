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

using DynamicBarriers = std::vector<std::pair<Circle, Point>>;

class TickState {
public:
    using Occupier = std::pair<bool, Circle>;

    TickState(DynamicBarriers dynamic_barriers,
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

    const DynamicBarriers& dynamic_barriers() const {
        return dynamic_barriers_;
    }

private:
    DynamicBarriers dynamic_barriers_;
    Occupier occupier_;
    double max_distance_error_;
};

class StepState {
public:
    StepState(double priority, double cost, double tick, PointInt position)
        : priority_(priority), cost_(cost), tick_(tick), position_(position) {}

    double priority() const {
        return priority_;
    }

    double cost() const {
        return cost_;
    }

    double tick() const {
        return tick_;
    }

    const PointInt& position() const {
        return position_;
    }

private:
    double priority_;
    double cost_;
    double tick_;
    PointInt position_;
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
                                    const DynamicBarriers& barriers) {
    return barriers.end() != std::find_if(barriers.begin(), barriers.end(),
        [&] (const auto& v) { return v.first.has_intersection(v.second, barrier, final_position); });
}

auto get_closest_circle(const std::vector<Circle>& barriers, const Line& path) {
    return std::min_element(barriers.begin(), barriers.end(),
         [&] (auto lhs, auto rhs) { return path.distance(lhs.position()) < path.distance(rhs.position()); });
}

double get_distance_to_closest_circle(const std::vector<Circle>& barriers, const Line& path) {
    return path.distance(get_closest_circle(barriers, path)->position());
}

auto get_closest_dynamic_barrier(const DynamicBarriers& barriers, const Line& path) {
    return std::min_element(barriers.begin(), barriers.end(),
         [&] (auto lhs, auto rhs) { return path.distance(lhs.second) < path.distance(rhs.second); });
}

double get_distance_to_closest_dynamic_barrier(const DynamicBarriers& barriers, const Line& path) {
    return path.distance(get_closest_dynamic_barrier(barriers, path)->second);
}

class GetOptimalPathImpl {
public:
    GetOptimalPathImpl(const Context& context, const Point& target, int step_size, Tick max_ticks, std::size_t max_iterations);

    Path operator ()();

private:
    const Context& context;
    const Point target;
    const int step_size;
    const Tick max_ticks;
    const std::size_t max_iterations;
    const Point initial_position = get_position(context.self());
    const Point global_shift = initial_position.to_int().to_double() - initial_position;
    const PointInt initial_position_int = (initial_position - global_shift).to_int();
    const double max_range = target.distance(initial_position) + step_size;
    const double speed = (context.game().getWizardForwardSpeed() + context.game().getWizardBackwardSpeed()
                          + 2 * context.game().getWizardStrafeSpeed()) / 4;

    const std::array<PointInt, 8> shifts = {{
        PointInt(step_size, 0),
        PointInt(step_size, step_size),
        PointInt(0, step_size),
        PointInt(-step_size, 0),
        PointInt(-step_size, -step_size),
        PointInt(0, -step_size),
        PointInt(step_size, -step_size),
        PointInt(-step_size, step_size),
    }};

    std::vector<const model::Projectile*> projectiles;
    std::vector<const model::Minion*> minions;
    std::vector<const model::Wizard*> wizards;

    std::vector<Circle> static_barriers;

    std::unordered_map<double, TickState> ticks_states {{
        {0, make_tick_state(0, 0)},
    }};

    Point shifted(const PointInt& position) const;
    TickState make_tick_state(double prev_tick, double tick) const;
    bool has_intersection(const PointInt& position, const TickState& tick_state, const PointInt& next_position) const;
    double get_distance_to_closest_barrier(const Line& path, const DynamicBarriers& dynamic_barriers) const;
    const TickState& get_tick_state(double prev_tick, double tick);
    double get_priority(const StepState& step_state, const PointInt& next_position);
    double get_base_priority(const PointInt& position) const;
    double get_cost(const StepState& step_state, const PointInt& next_position) const;
    double get_next_tick(const StepState& step_state, const PointInt& next_position) const;
    Path reconstruct_path(PointInt position, const std::map<PointInt, PointInt>& came_from) const;
};

GetOptimalPathImpl::GetOptimalPathImpl(const Context& context, const Point& target, int step_size, Tick max_ticks, std::size_t max_iterations)
        : context(context), target(target), step_size(step_size), max_ticks(max_ticks), max_iterations(max_iterations) {
    const IsInMyRange is_in_my_range {context, max_range};

    const auto initial_filter = [&] (const auto& units) {
        return filter_units(units, [&] (const auto& unit) { return !is_me(unit) && is_in_my_range(unit); });
    };

    projectiles = initial_filter(context.world().getProjectiles());
    minions = initial_filter(context.world().getMinions());
    wizards = initial_filter(context.world().getWizards());

    const auto buildings = initial_filter(context.world().getBuildings());
    const auto trees = initial_filter(context.world().getTrees());

    static_barriers.reserve(buildings.size() + trees.size());
    std::transform(buildings.begin(), buildings.end(), std::back_inserter(static_barriers), make_circle);
    std::transform(trees.begin(), trees.end(), std::back_inserter(static_barriers), make_circle);
}

Point GetOptimalPathImpl::shifted(const PointInt& position) const {
    return position.to_double() + global_shift;
}

TickState GetOptimalPathImpl::make_tick_state(double prev_tick, double tick) const {
    const auto make_circle = [&] (auto unit) {
        return std::make_pair(Circle(get_position(*unit) + get_speed(*unit) * prev_tick, unit->getRadius()),
                              get_position(*unit) + get_speed(*unit) * tick);
    };

    std::vector<std::pair<Circle, Point>> dynamic_barriers;
    dynamic_barriers.reserve(projectiles.size() + minions.size() + wizards.size());
    std::transform(projectiles.begin(), projectiles.end(), std::back_inserter(dynamic_barriers), make_circle);
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
}

bool GetOptimalPathImpl::has_intersection(const PointInt& position, const TickState& tick_state, const PointInt& next_position) const {
    const Circle barrier(shifted(position), context.self().getRadius());
    return (shifted(position).distance(initial_position) > max_range
            && shifted(position).distance(target) > max_range)
            || has_intersection_with_borders(barrier, context.game().getMapSize())
            || has_intersection_with_barriers(barrier, shifted(next_position), static_barriers)
            || has_intersection_with_barriers(barrier, shifted(next_position), tick_state.dynamic_barriers());
}

double GetOptimalPathImpl::get_distance_to_closest_barrier(const Line& path, const DynamicBarriers& dynamic_barriers) const {
    double result = std::numeric_limits<double>::max();

    if (!static_barriers.empty()) {
        result = std::min(result, get_distance_to_closest_circle(static_barriers, path));
    }

    if (!dynamic_barriers.empty()) {
        result = std::min(result, get_distance_to_closest_dynamic_barrier(dynamic_barriers, path));
    }

    return result;
}

const TickState& GetOptimalPathImpl::get_tick_state(double prev_tick, double tick) {
    auto tick_state = ticks_states.find(tick);
    if (tick_state == ticks_states.end()) {
        tick_state = ticks_states.insert({tick, make_tick_state(prev_tick, tick)}).first;
    }
    return tick_state->second;
}

double GetOptimalPathImpl::get_next_tick(const StepState& step_state, const PointInt& next_position) const {
    const auto length = step_state.position().distance(next_position);
    return step_state.tick() + length / speed;
}

double GetOptimalPathImpl::get_priority(const StepState& step_state, const PointInt& next_position) {
    const auto next_tick = get_next_tick(step_state, next_position);
    const auto tick_state = get_tick_state(step_state.tick(), next_tick);

    if (has_intersection(step_state.position(), tick_state, next_position)) {
        return std::numeric_limits<double>::max();
    }

    const Line trajectory(shifted(step_state.position()), shifted(next_position));
    const auto min_distance_to_barrier = get_distance_to_closest_barrier(trajectory, tick_state.dynamic_barriers());

    const auto base_priority = get_base_priority(next_position);

    return base_priority - bool(min_distance_to_barrier < 1) * 1000;
}

double GetOptimalPathImpl::get_base_priority(const PointInt& position) const {
    return - target.distance(shifted(position));
}

double GetOptimalPathImpl::get_cost(const StepState& step_state, const PointInt& next_position) const {
    const auto distance = step_state.position().distance(next_position);
    return step_state.cost() + distance;
}

Path GetOptimalPathImpl::reconstruct_path(PointInt position, const std::map<PointInt, PointInt>& came_from) const {
    const auto target_shift = target - target.to_int().to_double();
    const auto target_int = target.to_int();
    Path result;
    std::set<PointInt> visited;
    result.reserve(came_from.size());
    while (true) {
        if (!visited.insert(position).second) {
            break;
        }
        result.push_back(position.to_double() + (position == target_int ? target_shift : global_shift));
        const auto prev = came_from.find(position);
        if (prev == came_from.end()) {
            break;
        }
        position = prev->second;
    }
    std::reverse(result.begin(), result.end());
    return result;
}

Path GetOptimalPathImpl::operator ()() {
    const auto target_int = target.to_int();

    std::set<PointInt> closed;
    std::set<PointInt> opened({initial_position_int});
    std::map<PointInt, PointInt> came_from;
    std::map<PointInt, double> costs;
    std::priority_queue<StepState, std::deque<StepState>> queue;

    auto max_priority = get_base_priority(initial_position_int);

    costs[initial_position_int] = 0;

    queue.push(StepState(max_priority, 0, 0, initial_position_int));

    PointInt final_position;
    std::size_t iterations = 0;
    std::size_t current_max_iterations = max_iterations;
    const auto time_limit = context.time_limit();
    Duration max_duration(0);

    while (!queue.empty()) {
        context.check_timeout(__PRETTY_FUNCTION__, __FILE__, __LINE__);

        const auto iteration_start = Clock::now();

        const StepState step_state = queue.top();
        const TickState& tick_state = ticks_states.at(step_state.tick());

        if (shifted(step_state.position()).distance(target) <= tick_state.max_distance_error()) {
            if (!tick_state.occupier().first && target != shifted(step_state.position()) && !came_from.count(target_int)) {
                const auto it = came_from.find(step_state.position());
                if (it != came_from.end()) {
                    came_from[target_int] = it->second;
                    final_position = target_int;
                    break;
                }
            }
            final_position = step_state.position();
            break;
        }

        if (max_priority < step_state.priority()) {
            max_priority = step_state.priority();
            final_position = step_state.position();
        }

        if (++iterations >= current_max_iterations) {
            break;
        }

        queue.pop();
        opened.erase(step_state.position());
        closed.insert(step_state.position());

        if (step_state.tick() > max_ticks) {
            continue;
        }

        for (std::size_t i = 0; i < shifts.size() + 1; ++i) {
            context.check_timeout(__PRETTY_FUNCTION__, __FILE__, __LINE__);

            const auto shift = i == 0 ? target_int - step_state.position() : shifts[i - 1];
            const auto next_position = step_state.position() + shift;

            if (i != 0 && closed.count(next_position)) {
                continue;
            }

            const auto priority = get_priority(step_state, next_position);

            if (priority == std::numeric_limits<double>::max()) {
                continue;
            }

            const auto cost = get_cost(step_state, next_position);

            if (opened.insert(next_position).second) {
                queue.push(StepState(priority, cost, get_next_tick(step_state, next_position), next_position));
            } else if (cost >= costs.at(next_position)) {
                continue;
            }

            came_from[next_position] = step_state.position();
            costs[next_position] = cost;
        }

        if (max_iterations != std::numeric_limits<std::size_t>::max() && time_limit != Duration::max()) {
            const auto iteration_finish = Clock::now();
            max_duration = std::max(max_duration, Duration(iteration_finish - iteration_start));
            if (iterations > 3) {
                const std::size_t available_iterations = std::floor(std::max(time_limit.count() - 1, 0.0) / Duration(max_duration).count());
                current_max_iterations = std::min(current_max_iterations, available_iterations);
            }
        }
    }

    return reconstruct_path(final_position, came_from);
}

Path GetOptimalPath::operator ()(const Context& context, const Point& target) const {
    return GetOptimalPathImpl(context, target, step_size_, max_ticks_, max_iterations_)();
}

GetOptimalPath& GetOptimalPath::step_size(int value) {
    step_size_ = value;
    return *this;
}

GetOptimalPath& GetOptimalPath::max_ticks(Tick value) {
    max_ticks_ = value;
    return *this;
}

GetOptimalPath& GetOptimalPath::max_iterations(std::size_t value) {
    max_iterations_ = value;
    return *this;
}

}
