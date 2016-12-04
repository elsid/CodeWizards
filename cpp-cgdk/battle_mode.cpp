#include "battle_mode.hpp"
#include "optimal_target.hpp"
#include "optimal_position.hpp"

namespace strategy {

BattleMode::Result BattleMode::apply(const Context& context) {
    update_target(context);

    return target_.is_some() && destination_.first ? Result(target_, destination_.second) : Result();
}

void BattleMode::update_target(const Context& context) {
    destination_.first = false;
    target_ = get_optimal_target(context, 1.3 * context.self().getVisionRange());
    if (!target_.is_some()) {
        return;
    }
    destination_ = {true, get_optimal_position(context, target_, 2 * context.self().getVisionRange(),
                    OPTIMAL_POSITION_INITIAL_POINTS_COUNT, OPTIMAL_POSITION_MINIMIZE_MAX_FUNCTION_CALLS)};
}

}
