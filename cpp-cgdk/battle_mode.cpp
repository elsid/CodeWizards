#include "battle_mode.hpp"
#include "optimal_target.hpp"
#include "optimal_position.hpp"

namespace strategy {

BattleMode::Result BattleMode::apply(const Context& context) {
    update_target(context);

    return target_.is_some() && destination_.first ? Result(target_, destination_.second) : Result();
}

void BattleMode::reset() {
}

void BattleMode::update_target(const Context& context) {
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

}
