#pragma once

#include <type_traits>

#if defined(ELSID_STRATEGY_DEBUG) || defined(ELSID_STRATEGY_DEBUG_LOG)

#include "debug/output.hpp"

#include <iostream>

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

template <class Lhs, class Rhs>
inline typename std::enable_if<!std::is_same<Lhs, Rhs>::value, bool>::type operator ==(Id<Lhs>, Id<Rhs>) {
    return false;
}

template <class Lhs, class Rhs>
inline typename std::enable_if<std::is_same<Lhs, Rhs>::value, bool>::type operator ==(Id<Lhs> lhs, Id<Rhs> rhs) {
    return lhs.value() == rhs.value();
}

constexpr Tick MESSAGE_TICKS = 20000;
constexpr Tick OPTIMAL_PATH_MAX_TICKS = 100;
constexpr Tick OPTIMAL_PATH_MAX_ITERATIONS = 1000;
constexpr double OPTIMAL_POSITION_PRECISION = 1e-3;
constexpr long OPTIMAL_POSITION_MINIMIZE_MAX_FUNCTION_CALLS = 1000;
constexpr Tick BATTLE_MODE_TICKS = 2500;
constexpr Tick TICKS_TO_DEATH_FOR_RETREAT = 150;
constexpr Tick INACTIVE_TIMEOUT = 100;
constexpr Tick BONUSES_SPAWN_PERIOD = 2500;
constexpr Tick MINIONS_SPAWN_PERIOD = 750;

#if defined(ELSID_STRATEGY_DEBUG) || defined(ELSID_STRATEGY_DEBUG_LOG)

#define SLOG(context) std::cout << '[' << context.world().getTickIndex() << "] "

#else

struct DevNull {
    static DevNull& instance() {
        static DevNull value;
        return value;
    }
};

template <class T>
DevNull& operator <<(DevNull& stream, const T&) {
    return stream;
}

#define SLOG(context) ((void) context, strategy::DevNull::instance())

#endif

}
