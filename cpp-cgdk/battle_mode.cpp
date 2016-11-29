#include "battle_mode.hpp"
#include "optimal_target.hpp"
#include "optimal_position.hpp"

namespace strategy {

BattleMode::Result BattleMode::apply(const Context& context) {
    update_target(context);

    return target_.is_some() && destination_.first ? Result(target_, destination_.second) : Result();
}

void BattleMode::update_target(const Context& context) {
    const GetAttackRange get_attack_range {context};

    const auto max_distances = {
        1.3 * context.self().getVisionRange(),
        context.self().getCastRange() + context.game().getMagicMissileRadius(),
    };
    target_ = Target();
    destination_.first = false;
    for (const auto max_distance : max_distances) {
        const auto target = get_optimal_target(context, max_distance);
        const auto unit = target.circular_unit(context.cache());
        if (!unit) {
            break;
        }
        destination_ = {true, get_optimal_position(context, target, 2 * context.self().getVisionRange(),
                        OPTIMAL_POSITION_INITIAL_POINTS_COUNT, OPTIMAL_POSITION_MINIMIZE_MAX_FUNCTION_CALLS)};
        const auto max_attack_range = get_attack_range(context.self()) + unit->getRadius();
        const auto min_distance = std::min(destination_.second.distance(get_position(*unit)),
                                           get_position(context.self()).distance(get_position(*unit)));
        if (min_distance <= max_attack_range) {
            target_ = target;
            break;
        }
    }
}

}
