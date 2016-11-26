#pragma once

#include "mode.hpp"

namespace strategy {

class BattleMode : public Mode {
public:
    Result apply(const Context& context) override final;

private:
    Target target_;
    Point destination_;

    void update_target(const Context& context);
};

}
