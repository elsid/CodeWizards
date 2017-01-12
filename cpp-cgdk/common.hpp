#pragma once

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
const int OPTIMAL_PATH_STEP_SIZE = 5;
const Tick OPTIMAL_PATH_MAX_TICKS = 50;
const Tick OPTIMAL_PATH_MAX_ITERATIONS = 1000;
const double OPTIMAL_POSITION_PRECISION = 1e-3;
const long OPTIMAL_POSITION_MINIMIZE_MAX_FUNCTION_CALLS = 1000;
const Tick BATTLE_MODE_TICKS = 2500;
const Tick TICKS_TO_DEATH_FOR_RETREAT = 150;
const Tick INACTIVE_TIMEOUT = 100;

#if defined(ELSID_STRATEGY_DEBUG) || defined(ELSID_STRATEGY_DEBUG_LOG)

#define SLOG(context) std::cout << '[' << context.world().getTickIndex() << "] "

#else

struct DevNull {};

template <class T>
DevNull operator <<(DevNull, const T&) {
    return DevNull();
}

#define SLOG(context) (void) context, DevNull()

#endif

}
