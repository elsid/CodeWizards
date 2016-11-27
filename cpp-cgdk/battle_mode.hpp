#ifndef STRATEGY_BATTLE_MODE
#define STRATEGY_BATTLE_MODE

#include "mode.hpp"

namespace strategy {

class BattleMode : public Mode {
public:
    Result apply(const Context& context) override final;

private:
    Target target_;
    std::pair<bool, Point> destination_;

    void update_target(const Context& context);
};

}

#endif
