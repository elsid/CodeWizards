#pragma once

#include "common.hpp"

#include <algorithm>
#include <unordered_map>
#include <vector>

namespace strategy {

template <class T>
class CachedUnit {
public:
    using Value = T;

    CachedUnit(const Value& value, int tick)
        : value_(value), last_seen_(tick) {}

    const auto& value() const {
        return value_;
    }

    Tick last_seen() const {
        return last_seen_;
    }

private:
    Value value_;
    Tick last_seen_;
};

template <class T>
CachedUnit<T> make_cached(const T& unit, int tick) {
    return CachedUnit<T>(unit, tick);
}

template <class T>
class Cache {
public:
    using Units = std::unordered_map<UnitId, CachedUnit<T>>;

    const Units& units() const {
        return units_;
    }

    void update(const std::vector<T>& units, Tick tick) {
        std::transform(units.begin(), units.end(), std::inserter(units_, units_.end()),
                       [&] (const auto& unit) { return std::make_pair(unit.getId(), make_cached(unit, tick)); });
    }

private:
    Units units_;
};

}
