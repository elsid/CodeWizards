#pragma once

#include <simulation/simulation_strategy.hpp>
#include <battle_mode.hpp>

namespace strategy {
namespace simulation {
namespace scripts {

class TwoWizardsFightNearBonus {
public:
    TwoWizardsFightNearBonus(const Context& context);

    void apply(Context& context);

private:
    SimulationStrategy strategy_;
    BattleMode battle_mode_;
    Target target_;
    Point destination_;
};

} // namespace scripts
} // namespace simulation
} // namespace strategy
