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

const Tick MESSAGE_TICKS = 500;
const int OPTIMAL_PATH_STEP_SIZE = 20;
const Tick OPTIMAL_PATH_MAX_TICKS = 100;
const int OPTIMAL_POSITION_MINIMIZE_MAX_FUNCTION_CALLS = 100;
const std::size_t OPTIMAL_POSITION_INITIAL_POINTS_COUNT = 3;

}

#endif
