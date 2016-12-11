#include "battle_mode.hpp"
#include "optimal_target.hpp"
#include "optimal_position.hpp"

namespace strategy {

BattleMode::Result BattleMode::apply(const Context& context) {
    update_target(context);

    return target_.is_some() && destination_.first ? Result(target_, destination_.second) : Result();
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
        destination_ = {true, get_optimal_position(context, target_, 2 * context.self().getVisionRange(),
                        OPTIMAL_POSITION_INITIAL_POINTS_COUNT, OPTIMAL_POSITION_MINIMIZE_MAX_FUNCTION_CALLS)};
        break;
    }
}

}
