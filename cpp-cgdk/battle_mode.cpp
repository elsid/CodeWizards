#include "battle_mode.hpp"
#include "optimal_target.hpp"
#include "optimal_position.hpp"

namespace strategy {

inline Circle make_circle(const model::CircularUnit& unit) {
    return Circle(get_position(unit), unit.getRadius());
}

BattleMode::Result BattleMode::apply(const Context& context) {
    update_target(context);

    return destination_.first ? Result(target_, destination_.second) : Result();
}

void BattleMode::reset() {
}

void BattleMode::update_target(const Context& context) {
    if (is_under_fire(context)) {
        target_ = Target();
        points_.clear();
        destination_ = {true, this->get_optimal_position(context)};
        return;
    }

    const auto max_distances = {
        context.game().getStaffRange(),
        get_max_distance_for_unit_candidate(context),
    };

    destination_.first = false;

    for (const auto max_distance : max_distances) {
        target_ = get_optimal_target(context, max_distance);

        if (!target_.is_some()) {
            continue;
        }

        target_.apply(context.cache(), [&] (auto target) {
            if (target) {
                points_.clear();
                destination_ = {true, this->get_optimal_position(context, target)};
            }
        });

        if (destination_.first) {
            break;
        }
    }

    if (!destination_.first || will_cast_later(context)) {
        target_ = Target();
        points_.clear();
        destination_ = {true, this->get_optimal_position(context)};
    }

    const auto me = make_circle(context.self());
    const auto my_position = get_position(context.self());
    const auto distance_to_destination = my_position.distance(destination_.second);
    auto closest_distance = target_.is_some()
            ? target_.apply(context.cache(), [&] (auto unit) {
                return unit ? std::min(get_position(*unit).distance(my_position), distance_to_destination) : distance_to_destination;
            })
            : distance_to_destination;
    const model::Tree* closest_tree_barrier = nullptr;
    for (const auto& tree : context.world().getTrees()) {
        const auto distance = get_position(tree).distance(my_position);
        if (closest_distance > distance && make_circle(tree).has_intersection(me, destination_.second)) {
            closest_tree_barrier = &tree;
            closest_distance = distance;
        }
    }

    if (closest_tree_barrier) {
        target_ = Target(get_id(*closest_tree_barrier));
        target_.apply(context.cache(), [&] (auto unit) {
            if (unit) {
                points_.clear();
                destination_ = {true, this->get_optimal_position(context, unit)};
            }
        });
    }
}

bool BattleMode::is_under_fire(const Context& context) const {
    const GetProjectileTrajectory get_projectile_trajectory {context};

    const auto can_reach_me = [&] (const auto& v) {
        const auto& cached_unit = v.second;

        if (cached_unit.value().getFaction() == context.self().getFaction()) {
            return false;
        }

        const auto trajectory = get_projectile_trajectory(cached_unit);
        const auto nearest = trajectory.nearest(get_position(context.self()));
        const auto my_position = get_position(context.self());

        double distance;

        if (trajectory.has_point(nearest)) {
            distance = trajectory.distance(my_position);
        } else {
            distance = std::min(trajectory.begin().distance(my_position), trajectory.end().distance(my_position));
        }

        return distance < get_projectile_explosion_radius(cached_unit.value().getType(), context.game())
                + context.self().getRadius() + 1;
    };

    const auto& projectiles = get_units<model::Projectile>(context.cache());

    return projectiles.end() != std::find_if(projectiles.begin(), projectiles.end(), can_reach_me);
}

bool BattleMode::will_cast_later(const Context& context) const {
    const GetMaxDamage get_max_damage {context};
    const auto distance = target_distance(context);
    const auto ticks = get_max_damage.next_attack_action(context.self(), distance).second;
    return distance > 2 * context.game().getStaffRange() && ticks > context.game().getWizardActionCooldownTicks();
}

double BattleMode::target_distance(const Context& context) const {
    if (!target_.is_some()) {
        return std::numeric_limits<double>::max();
    }

    return target_.apply(context.cache(), [&] (auto unit) {
        if (unit) {
            return get_position(context.self()).distance(get_position(*unit));
        } else {
            return std::numeric_limits<double>::max();
        }
    });
}

template <class TargetT>
Point BattleMode::get_optimal_position(const Context& context, const TargetT* target) {
    return GetOptimalPosition<TargetT>()
            .target(target)
            .max_distance(get_max_distance_for_optimal_position(context))
            .precision(OPTIMAL_POSITION_PRECISION)
            .max_function_calls(OPTIMAL_POSITION_MINIMIZE_MAX_FUNCTION_CALLS)
#ifdef ELSID_STRATEGY_DEBUG
            .points(&points_)
#endif
            (context);
}

Point BattleMode::get_optimal_position(const Context&, const model::Bonus* target) {
    return get_position(*target);
}

Point BattleMode::get_optimal_position(const Context& context) {
    return GetOptimalPosition<model::LivingUnit>()
            .max_distance(get_max_distance_for_optimal_position(context))
            .precision(OPTIMAL_POSITION_PRECISION)
            .max_function_calls(OPTIMAL_POSITION_MINIMIZE_MAX_FUNCTION_CALLS)
#ifdef ELSID_STRATEGY_DEBUG
            .points(&points_)
#endif
            (context);
}

}
