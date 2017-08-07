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

bool has_intersection_with_borders(const Circle& circle, double map_size) {
    const double delta = 1e-8;
    const auto left = circle.position().x() - circle.radius();
    const auto top = circle.position().y() - circle.radius();
    const auto right = map_size - circle.radius() - circle.position().x();
    const auto bottom = map_size - circle.radius() - circle.position().y();
    return left <= delta || top <= delta || right <= delta || bottom <= delta;
}

class GetOptimalPathImpl {
public:
    GetOptimalPathImpl(const Context& context, const Point& target, double step_size, Tick max_ticks, std::size_t max_iterations);

    Path operator ()();

    const std::unordered_map<int, TickState>& get_ticks_states() const {
        return ticks_states;
    }

    const std::vector<StepState>& get_steps_states() const {
        return steps_states;
    }

private:
    struct GreaterByPriority {
        bool operator ()(const StepState& lhs, const StepState& rhs) const {
            return lhs.priority() > rhs.priority();
        }
    };

    using Queue = std::priority_queue<StepState, std::deque<StepState>, GreaterByPriority>;

    const Context& context;
    const Point target;
    const double step_size;
    const Tick max_ticks;
    const std::size_t max_iterations;
    const Point initial_position = get_position(context.self());
    const double max_range = context.self().getVisionRange();
    const double speed = (context.game().getWizardForwardSpeed() + context.game().getWizardBackwardSpeed()
                          + 2 * context.game().getWizardStrafeSpeed()) / 4;

    std::vector<const model::Projectile*> projectiles;
    std::vector<const model::Minion*> minions;
    std::vector<const model::Wizard*> wizards;

    std::vector<Circle> static_barriers;

    std::unordered_map<int, TickState> ticks_states;
    std::vector<StepState> steps_states;

    Queue queue;
    std::map<PointInt, double> costs;
    std::set<std::pair<PointInt, PointInt>> pushed;
    std::map<Point, StepState> came_from;

    TickState make_tick_state(double prev_tick, double tick) const;
    const TickState& get_tick_state(double prev_tick, double tick);
    double get_priority(const Point& position) const;
    double get_next_cost(const StepState& step_state) const;
    double get_next_tick(const StepState& step_state, const Point& next_position) const;
    Path reconstruct_path(Point position, const std::map<Point, StepState>& came_from) const;
    void fill_steps_states(StepState step_state, const std::map<Point, StepState>& came_from);
    Point adjust_target(const Point& position, const Point& target) const;
    void add_state(const StepState& step_state, const Point& next_target);
    const Circle* get_closest_dynamic_barrier(const StepState& step_state);
};

GetOptimalPathImpl::GetOptimalPathImpl(const Context& context, const Point& target, double step_size, Tick max_ticks, std::size_t max_iterations)
        : context(context), target(target), step_size(step_size), max_ticks(max_ticks), max_iterations(max_iterations) {
    const IsInMyRange is_projectile_in_my_range {context, context.self().getVisionRange()};
    const IsInMyRange is_in_my_range {context, max_range};

    const auto initial_projectiles_filter = [&] (const auto& units) {
        return filter_units(units, [&] (const auto& unit) { return is_projectile_in_my_range(unit); });
    };

    const auto initial_filter = [&] (const auto& units) {
        return filter_units(units, [&] (const auto& unit) { return !is_me(unit) && is_in_my_range(unit); });
    };

    projectiles = initial_projectiles_filter(context.world().getProjectiles());
    minions = initial_filter(context.world().getMinions());
    wizards = initial_filter(context.world().getWizards());

    const auto buildings = initial_filter(context.world().getBuildings());
    const auto trees = initial_filter(context.world().getTrees());

    static_barriers.reserve(buildings.size() + trees.size());
    std::transform(buildings.begin(), buildings.end(), std::back_inserter(static_barriers), make_circle);
    std::transform(trees.begin(), trees.end(), std::back_inserter(static_barriers), make_circle);
    ticks_states.insert({0, make_tick_state(0, 0)});
}

TickState GetOptimalPathImpl::make_tick_state(double prev_tick, double tick) const {
    const auto make = [&] (auto unit) {
        return DynamicBarrier {
            std::size_t(unit),
            Circle(get_position(*unit) + get_speed(*unit) * prev_tick, unit->getRadius()),
            get_position(*unit) + get_speed(*unit) * tick,
        };
    };

    DynamicBarriers dynamic_barriers;
    dynamic_barriers.reserve(projectiles.size() + minions.size() + wizards.size());
    std::transform(projectiles.begin(), projectiles.end(), std::back_inserter(dynamic_barriers), make);
    std::transform(minions.begin(), minions.end(), std::back_inserter(dynamic_barriers), make);
    std::transform(wizards.begin(), wizards.end(), std::back_inserter(dynamic_barriers), make);

    const Circle barrier(get_position(context.self()), context.self().getRadius());

    TickState::Occupier occupier;
    const auto static_occupier = std::find_if(static_barriers.begin(), static_barriers.end(),
        [&] (const auto& v) { return v.position().distance(target) <= v.radius() + barrier.radius(); });
    if (static_occupier != static_barriers.end()) {
        occupier = {true, *static_occupier};
    } else {
        const auto dynamic_occupier = std::find_if(dynamic_barriers.begin(), dynamic_barriers.end(),
            [&] (const auto& v) { return v.target.distance(target) <= v.circle.radius() + barrier.radius(); });
        if (dynamic_occupier != dynamic_barriers.end()) {
            occupier = {true, dynamic_occupier->circle};
        }
    }

    const auto max_distance_error = occupier.first ? occupier.second.radius() + barrier.radius() + 1 : 1;

    return TickState(std::move(dynamic_barriers), occupier, max_distance_error);
}

const TickState& GetOptimalPathImpl::get_tick_state(double prev_tick, double tick) {
    auto tick_state = ticks_states.find(std::round(tick));
    if (tick_state == ticks_states.end()) {
        tick_state = ticks_states.insert({std::round(tick), make_tick_state(prev_tick, tick)}).first;
    }
    return tick_state->second;
}

double GetOptimalPathImpl::get_next_tick(const StepState& step_state, const Point& next_position) const {
    const auto length = step_state.position().distance(next_position);
    return step_state.tick() + length / speed;
}

double GetOptimalPathImpl::get_priority(const Point& position) const {
    return target.distance(position);
}

double GetOptimalPathImpl::get_next_cost(const StepState& step_state) const {
    const auto distance = step_state.position().distance(step_state.target());
    return step_state.cost() + distance;
}

Path GetOptimalPathImpl::reconstruct_path(Point position, const std::map<Point, StepState>& came_from) const {
    Path result;
    std::set<Point> visited;
    result.reserve(came_from.size());
    while (true) {
        if (!visited.insert(position).second) {
            break;
        }
        result.push_back(position);
        const auto prev = came_from.find(position);
        if (prev == came_from.end()) {
            break;
        }
        position = prev->second.position();
    }
    std::reverse(result.begin(), result.end());
    return result;
}

void GetOptimalPathImpl::fill_steps_states(StepState step_state, const std::map<Point, StepState>& came_from) {
    steps_states.clear();
    steps_states.reserve(came_from.size());
    std::set<Point> visited;
    while (true) {
        if (!visited.insert(step_state.position()).second) {
            break;
        }
        steps_states.push_back(step_state);
        const auto prev = came_from.find(step_state.position());
        if (prev == came_from.end()) {
            break;
        }
        step_state = prev->second;
    }
    std::reverse(steps_states.begin(), steps_states.end());
}

std::pair<Point, Point> get_tangent_points(const Circle& circle, const Point& source) {
    const auto hypot = circle.position().distance(source);
    const auto far_cathetus = circle.radius();
    const auto near_cathetus = std::sqrt(math::square(hypot) - math::square(far_cathetus));
    const auto sin = far_cathetus / hypot;
    const auto angle = std::asin(sin);
    const auto to_circle = (circle.position() - source).normalized();
    const auto to_left_tangent = source + to_circle.rotated(angle) * near_cathetus;
    const auto to_right_tangent = source + to_circle.rotated(-angle) * near_cathetus;
    return {to_left_tangent, to_right_tangent};
}

Point GetOptimalPathImpl::adjust_target(const Point& position, const Point& target) const {
    const auto to_target = target - position;
    const auto norm = to_target.norm();
    return position + to_target * (1 + step_size / norm);
}

void GetOptimalPathImpl::add_state(const StepState& step_state, const Point& next_target) {
    const Circle barrier(next_target, context.self().getRadius());

    if (has_intersection_with_borders(barrier, context.game().getMapSize())) {
        return;
    }

    if (!pushed.insert({step_state.position().to_int(), next_target.to_int()}).second) {
        return;
    }

    const auto other = costs.find(next_target.to_int());
    const auto distance = next_target.distance(target);

    if (other != costs.end() && other->second > step_state.cost() + distance) {
        return;
    }

    queue.push(StepState(distance, step_state.cost(), step_state.tick(), step_state.position(), next_target));
}

std::vector<Circle>::const_iterator get_closest_barrier(const Circle& my_barrier, const Point& target, const std::vector<Circle>& static_barriers) {
    auto closest = static_barriers.end();
    double closest_distance = std::numeric_limits<double>::max();
    for (auto barrier = static_barriers.begin(); barrier != static_barriers.end(); ++barrier) {
        if (barrier->has_intersection(my_barrier, target)) {
            const auto distance = barrier->position().distance(my_barrier.position());
            if (closest_distance > distance) {
                closest = barrier;
                closest_distance = distance;
            }
        }
    }
    return closest;
}

const Circle* GetOptimalPathImpl::get_closest_dynamic_barrier(const StepState& step_state) {
    const auto& tick_state = get_tick_state(step_state.tick(), get_next_tick(step_state, step_state.target()));
    const auto& dynamic_barriers = tick_state.dynamic_barriers();
    const Circle* result = nullptr;
    double closest_distance = std::numeric_limits<double>::max();

    for (auto barrier = dynamic_barriers.begin(); barrier != dynamic_barriers.end(); ++barrier) {
        const Circle my_barrier(step_state.position(), context.self().getRadius());

        if (!barrier->circle.has_intersection(barrier->target, my_barrier, step_state.target())) {
            continue;
        }

        if (barrier->circle.position() == barrier->target) {
            const auto distance = step_state.position().distance(barrier->target);
            if (closest_distance > distance) {
                closest_distance = distance;
                result = &barrier->circle;
            }
            continue;
        }

        const Line my_trajectory(my_barrier.position(), target);
        const Line dynamic_barrier_trajectory(barrier->circle.position(), barrier->target);
        bool has_intersection;
        Point intersection;
        std::tie(has_intersection, intersection) = my_trajectory.intersection(dynamic_barrier_trajectory);

        if (!has_intersection) {
            continue;
        }

        const auto distance = step_state.position().distance(intersection);
        const auto ticks_to_intersection = distance / speed;
        const auto tick = step_state.tick() + ticks_to_intersection;
        const auto& ticks_state = get_tick_state(step_state.tick(), tick);
        const auto& dynamic_barriers = ticks_state.dynamic_barriers();
        const auto barrier_at_tick = std::find_if(dynamic_barriers.begin(), dynamic_barriers.end(),
            [&] (const auto& v) { return v.id == barrier->id; });

        if (barrier_at_tick == dynamic_barriers.end()) {
            continue;
        }

        const auto to_target = (step_state.target() - step_state.position()).normalized();
        const auto my_position_at_tick = step_state.position() + ticks_to_intersection * speed * to_target;
        const Circle my_barrier_at_tick(my_position_at_tick, my_barrier.radius());

        if (!barrier_at_tick->circle.has_intersection(my_barrier_at_tick)) {
            continue;
        }

        if (closest_distance > distance) {
            closest_distance = distance;
            result = &barrier_at_tick->circle;
        }
    }

    return result;
}

Path GetOptimalPathImpl::operator ()() {
    std::size_t iterations = 0;
    const StepState initial_state(get_priority(initial_position), 0, 0, initial_position, target);
    auto final_state = initial_state;
    std::size_t current_max_iterations = max_iterations;
    const auto time_limit = context.time_limit();
    Duration max_duration(0);

    queue = Queue();
    costs.clear();
    pushed.clear();
    came_from.clear();

    queue.push(initial_state);

    while (!queue.empty()) {
        context.check_timeout(__PRETTY_FUNCTION__, __FILE__, __LINE__);

        const auto iteration_start = Clock::now();

        const auto step_state = queue.top();

        if (step_state.position() == target) {
            final_state = step_state;
            break;
        }

        if (final_state.priority() > step_state.priority()) {
            final_state = step_state;
        }

        if (++iterations > max_iterations) {
            break;
        }

        queue.pop();

        if (step_state.tick() > max_ticks) {
            continue;
        }

        const Circle my_barrier(step_state.position(), context.self().getRadius());
        const auto closest_static_barrier = strategy::get_closest_barrier(my_barrier, step_state.target(), static_barriers);
        const auto closest_dynamic_barrier = get_closest_dynamic_barrier(step_state);
        const Circle* closest = nullptr;

        if (closest_static_barrier != static_barriers.end() && closest_dynamic_barrier) {
            const auto distance_to_static = closest_static_barrier->position().distance(step_state.position());
            const auto distance_to_dynamic = closest_dynamic_barrier->position().distance(step_state.position());
            closest = distance_to_dynamic < distance_to_static ? closest_dynamic_barrier : &*closest_static_barrier;
        } else if (closest_static_barrier != static_barriers.end()) {
            closest = &*closest_static_barrier;
        } else if (closest_dynamic_barrier) {
            closest = closest_dynamic_barrier;
        }

        if (closest) {
            const Circle barrier(closest->position(), closest->radius() + context.self().getRadius() + 0.1);

            if (step_state.position().distance(barrier.position()) > barrier.radius()) {
                const auto tangents = get_tangent_points(barrier, step_state.position());

                add_state(step_state, adjust_target(step_state.position(), tangents.first));
                add_state(step_state, adjust_target(step_state.position(), tangents.second));
            } else if (iterations <= 1) {
                const auto from_barrier = (step_state.position() - closest->position()).normalized();
                const auto border = closest->position() + from_barrier * (closest->radius() + context.self().getRadius() + 1);
                const auto shift = from_barrier * step_size;

                add_state(step_state, border + shift);
                add_state(step_state, border + shift.left_orhtogonal());
                add_state(step_state, border - shift.left_orhtogonal());
            }
        } else {
            const auto other = costs.find(step_state.target().to_int());

            if (other == costs.end()) {
                costs.insert({step_state.target().to_int(), step_state.cost()});
            } else if (other->second > step_state.cost()) {
                other->second = step_state.cost();
            } else {
                continue;
            }

            if (pushed.insert({step_state.target().to_int(), target.to_int()}).second) {
                queue.push(StepState(0, get_next_cost(step_state), get_next_tick(step_state, step_state.target()), step_state.target(), target));

                if (step_state.target() != step_state.position()) {
                    came_from[step_state.target()] = step_state;
                }
            }
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

    fill_steps_states(final_state, came_from);

    return reconstruct_path(final_state.position(), came_from);
}

Path GetOptimalPath::operator ()(const Context& context, const Point& target) const {
    GetOptimalPathImpl impl(context, target, step_size_, max_ticks_, max_iterations_);
    const auto result = impl();

    if (ticks_states_) {
        const auto& src = impl.get_ticks_states();
        std::copy(src.begin(), src.end(), std::inserter(*ticks_states_, ticks_states_->end()));
    }

    if (steps_states_) {
        const auto& src = impl.get_steps_states();
        std::copy(src.begin(), src.end(), std::back_inserter(*steps_states_));
    }

    return result;
}

GetOptimalPath& GetOptimalPath::step_size(double value) {
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

GetOptimalPath& GetOptimalPath::ticks_states(std::map<int, TickState>* value) {
    ticks_states_ = value;
    return *this;
}

GetOptimalPath& GetOptimalPath::steps_states(std::vector<StepState>* value) {
    steps_states_ = value;
    return *this;
}

}
