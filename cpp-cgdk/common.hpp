#ifndef STRATEGY_COMMON_HPP
#define STRATEGY_COMMON_HPP

namespace strategy {

using UnitId = long long;
using Tick = int;

template <class T>
class Id {
public:
    using Type = T;

    Id(UnitId value = 0) : value_(value) {}

    UnitId value() const {
        return value_;
    }

private:
    UnitId value_;
};

const Tick MESSAGE_TICKS = 20000;
const int OPTIMAL_PATH_STEP_SIZE = 20;
const Tick OPTIMAL_PATH_MAX_TICKS = 50;
const Tick OPTIMAL_PATH_MAX_ITERATIONS = 1000;
const int OPTIMAL_POSITION_MINIMIZE_MAX_FUNCTION_CALLS = 200;
const std::size_t OPTIMAL_POSITION_INITIAL_POINTS_COUNT = 1;
const double PROJECTILE_PENALTY_WEIGHT = 1.1;
const double ELIMINATION_SCORE_WEIGHT = 1;
const double UNITS_DANGER_PENALTY_WEIGHT = 1;
const Tick BATTLE_MODE_TICKS = 2500;

}

#endif
