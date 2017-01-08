#pragma once

#include "battle_mode.hpp"
#include "move_mode.hpp"

#include <memory>

namespace strategy {

class RetreatMode : public Mode {
public:
    RetreatMode(std::shared_ptr<BattleMode> battle_mode, std::shared_ptr<MoveMode> move_mode);

    Result apply(const Context& context) override final;
    void reset() override final;

    const char* name() const override final {
        return "retreat";
    }

private:
    std::shared_ptr<BattleMode> battle_mode_;
    std::shared_ptr<MoveMode> move_mode_;
};

}
