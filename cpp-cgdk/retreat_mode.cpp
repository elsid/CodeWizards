#include "retreat_mode.hpp"

namespace strategy {

RetreatMode::RetreatMode(std::shared_ptr<BattleMode> battle_mode, std::shared_ptr<MoveMode> move_mode)
    : battle_mode_(std::move(battle_mode)), move_mode_(std::move(move_mode)) {}

Mode::Result RetreatMode::apply(const Context& context) {
    const auto battle_result = battle_mode_->apply(context);
    const auto move_result = move_mode_->apply(context);

    if (battle_result.active() && move_result.active()) {
        return Result(battle_result.target(), move_result.destination());
    } else if (battle_result.active()) {
        return battle_result;
    } else {
        return move_result;
    }
}

void RetreatMode::reset() {
    move_mode_->reset();
}

}
