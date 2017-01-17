#pragma once

#if defined(ELSID_STRATEGY_DEBUG) || defined(ELSID_STRATEGY_DEBUG_LOG)

#include <iostream>

#else

#include <ostream>

#endif

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

constexpr Tick MESSAGE_TICKS = 20000;
constexpr int OPTIMAL_PATH_STEP_SIZE = 5;
constexpr Tick OPTIMAL_PATH_MAX_TICKS = 50;
constexpr Tick OPTIMAL_PATH_MAX_ITERATIONS = 1000;
constexpr double OPTIMAL_POSITION_PRECISION = 1e-3;
constexpr long OPTIMAL_POSITION_MINIMIZE_MAX_FUNCTION_CALLS = 1000;
constexpr Tick BATTLE_MODE_TICKS = 2500;
constexpr Tick TICKS_TO_DEATH_FOR_RETREAT = 150;
constexpr Tick INACTIVE_TIMEOUT = 100;

#if defined(ELSID_STRATEGY_DEBUG) || defined(ELSID_STRATEGY_DEBUG_LOG)

#define SLOG(context) std::cout << '[' << context.world().getTickIndex() << "] "

#else

struct DevNull : public std::ostream {
    static DevNull& instance() {
        static DevNull value;
        return value;
    }
};

template <class T>
DevNull& operator <<(DevNull& stream, const T&) {
    return stream;
}

#define SLOG(context) ((void) context, DevNull::instance())

#endif

}
