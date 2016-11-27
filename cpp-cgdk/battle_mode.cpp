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
        1.3 * context.self().getVisionRange(),
        context.self().getCastRange() + context.game().getMagicMissileRadius(),
    };
    for (const auto max_distance : max_distances) {
        destination_.first = false;
        target_ = get_optimal_target(context, max_distance);
        const auto unit = target_.circular_unit(context.cache());
        if (!unit) {
            break;
        }
        destination_ = {true, get_optimal_position(context, target_, 1.3 * context.self().getVisionRange())};
        if (destination_.second.distance(get_position(*unit))
                <= context.self().getCastRange() + context.game().getMagicMissileRadius() + unit->getRadius()) {
            break;
        }
    }
}

}
