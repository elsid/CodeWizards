#include "retreat_mode.hpp"
#include "helpers.hpp"

namespace strategy {

RetreatMode::RetreatMode(std::shared_ptr<BattleMode> battle_mode, std::shared_ptr<MoveMode> move_mode)
    : battle_mode_(std::move(battle_mode)), move_mode_(std::move(move_mode)) {}

Mode::Result RetreatMode::apply(const Context& context) {
    const auto battle_result = battle_mode_->apply(context);
    const auto move_result = move_mode_->apply(context);

    if (!battle_result.active()) {
        return move_result;
    }

    if (!battle_result.target().is_some()) {
        return battle_result;
    }

    const auto use_target = battle_result.target().apply(context.cache(), [&] (auto unit) {
        if (unit) {
            const auto distance = get_position(context.self()).distance(get_position(*unit));
            return distance <= context.game().getStaffRange() + unit->getRadius();
        } else {
            return false;
        }
    });

    if (use_target) {
        return Result(battle_result.target(), move_result.destination());
    } else {
        return move_result;
    }
}

void RetreatMode::reset() {
    move_mode_->reset();
}

}
