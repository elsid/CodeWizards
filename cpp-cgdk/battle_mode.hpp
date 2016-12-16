#ifndef STRATEGY_BATTLE_MODE_HPP
#define STRATEGY_BATTLE_MODE_HPP

#include "mode.hpp"

namespace strategy {

class BattleMode : public Mode {
public:
    Result apply(const Context& context) override final;
    void reset() override final;

private:
    Target target_;
    std::pair<bool, Point> destination_;

    void update_target(const Context& context);
};

}

#endif
