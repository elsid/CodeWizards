#ifndef STRATEGY_CACHE_HPP
#define STRATEGY_CACHE_HPP

#include "common.hpp"

#include "model/LivingUnit.h"

#include <algorithm>
#include <unordered_map>
#include <vector>
#include <numeric>

#include "point.hpp"

namespace strategy {

inline int get_life(const model::Unit&) {
    return 0;
}

inline int get_life(const model::LivingUnit& unit) {
    return unit.getLife();
}

template <class T>
class CachedUnit {
public:
    static constexpr std::size_t LIFE_CHANGES_SIZE = 5;
    static constexpr std::size_t SPEEDS_SIZE = 5;

    using Value = T;

    CachedUnit() = default;

    CachedUnit(const Value& value, Tick tick)
        : value_(value),
          first_seen_(tick),
          last_seen_(tick),
          prev_life_change_(tick),
          prev_life_(get_life(value)),
          first_position_(value.getX(), value.getY()) {
        speeds_.front() = Point(value.getSpeedX(), value.getSpeedY());
    }

    const Value& value() const {
        return value_;
    }

    Tick last_seen() const {
        return last_seen_;
    }

    const Point& first_position() const {
        return first_position_;
    }

    void set(const Value& value, Tick last_seen) {
        value_ = value;
        std::rotate(speeds_.rbegin(), speeds_.rbegin() + 1, speeds_.rend());
        speeds_.front() = Point(value.getSpeedX(), value.getSpeedY());
        last_seen_ = last_seen;
        if (const auto life_change = get_life(value) - prev_life_) {
            std::rotate(life_change_.rbegin(), life_change_.rbegin() + 1, life_change_.rend());
            life_change_.front() = {last_seen - prev_life_change_, life_change};
            prev_life_ = get_life(value);
            prev_life_change_ = last_seen;
        }
    }

    double mean_life_change_speed() const {
        const auto interval = std::accumulate(life_change_.begin(), life_change_.end(), 0,
            [&] (auto sum, const auto& v) { return sum + v.first; });
        const auto life_change = std::accumulate(life_change_.begin(), life_change_.end(), 0,
            [&] (auto sum, const auto& v) { return sum + v.second; });
        return double(life_change) / (interval ? double(interval) : 1);
    }

    Point mean_speed() const {
        const auto sum_speed = std::accumulate(speeds_.begin(), speeds_.end(), Point(0, 0), std::plus<Point>());
        return sum_speed / speeds_.size();
    }

private:
    Value value_;
    Tick first_seen_ = 0;
    Tick last_seen_ = 0;
    Tick prev_life_change_ = 0;
    int prev_life_ = 0;
    Point first_position_;
    std::array<std::pair<Tick, int>, LIFE_CHANGES_SIZE> life_change_;
    std::array<Point, SPEEDS_SIZE> speeds_;
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

    void update(const T& unit, Tick tick) {
        const auto it = units_.find(unit.getId());
        if (it == units_.end()) {
            units_[unit.getId()] = make_cached(unit, tick);
        } else {
            it->second.set(unit, tick);
        }
    }

    void update(const std::vector<T>& units, Tick tick) {
        for (const auto& unit : units) {
            update(unit, tick);
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

#endif
