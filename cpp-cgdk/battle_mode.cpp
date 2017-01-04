#include "battle_mode.hpp"
#include "optimal_target.hpp"
#include "optimal_position.hpp"

namespace strategy {

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
        get_max_distance_for_tree_candidate(context),
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
    return GetOptimalPosition<model::CircularUnit>()
            .max_distance(get_max_distance_for_optimal_position(context))
            .precision(OPTIMAL_POSITION_PRECISION)
            .max_function_calls(OPTIMAL_POSITION_MINIMIZE_MAX_FUNCTION_CALLS)
#ifdef ELSID_STRATEGY_DEBUG
            .points(&points_)
#endif
            (context);
}

}
