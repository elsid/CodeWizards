#ifndef STRATEGY_RETREAT_MODE_HPP
#define STRATEGY_RETREAT_MODE_HPP

#include "battle_mode.hpp"
#include "move_mode.hpp"

#include <memory>

namespace strategy {

class RetreatMode : public Mode {
public:
    RetreatMode(std::shared_ptr<BattleMode> battle_mode, std::shared_ptr<MoveMode> move_mode);

    Result apply(const Context& context) override final;
    void reset() override final;

private:
    std::shared_ptr<BattleMode> battle_mode_;
    std::shared_ptr<MoveMode> move_mode_;
};

}

#endif
