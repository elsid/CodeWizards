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

    CachedUnit() = default;

    CachedUnit(const Value& value, Tick tick)
        : value_(value), last_seen_(tick) {}

    const Value& value() const {
        return value_;
    }

    Tick last_seen() const {
        return last_seen_;
    }

private:
    Value value_;
    Tick last_seen_ = 0;
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
        for (const auto& unit : units) {
            units_[unit.getId()] = make_cached(unit, tick);
        }
    }

    template <class Predicate>
    void invalidate(const Predicate& predicate) {
        for (auto it = units_.begin(); it != units_.end();) {
            if (predicate(it->second)) {
                it = units_.erase(it);
            } else {
                ++it;
            }
        }
    }

private:
    Units units_;
};

}
