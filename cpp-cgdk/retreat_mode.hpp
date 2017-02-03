#pragma once

#include "battle_mode.hpp"
#include "move_mode.hpp"

namespace strategy {

class RetreatMode : public Mode {
public:
    RetreatMode(BattleMode& battle_mode, MoveMode& move_mode);

    Result apply(const Context& context) override final;
    void reset() override final;

private:
    BattleMode& battle_mode_;
    MoveMode& move_mode_;
};

}
