#include "battle_mode.hpp"
#include "optimal_target.hpp"
#include "optimal_position.hpp"

namespace strategy {

BattleMode::Result BattleMode::apply(const Context& context) {
    update_target(context);

    return target_.has_value() ? Result(target_, destination_) : Result();
}

void BattleMode::update_target(const Context& context) {
    const auto max_distances = {
        1.3 * context.self().getVisionRange(),
        context.self().getCastRange() + context.game().getMagicMissileRadius(),
    };
    for (const auto max_distance : max_distances) {
        target_ = get_optimal_target(context, max_distance);
        const auto unit = target_.unit();
        if (!unit) {
            break;
        }
        destination_ = get_optimal_position(context, target_, max_distance);
        if (destination_.distance(get_position(*unit))
                <= context.self().getCastRange() + context.game().getMagicMissileRadius() + unit->getRadius()) {
            break;
        }
    }
}

}
