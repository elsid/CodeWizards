#pragma once

#include "common.hpp"

#include "model/Bonus.h"
#include "model/Building.h"
#include "model/Minion.h"
#include "model/Projectile.h"
#include "model/Tree.h"
#include "model/Wizard.h"

#include <algorithm>
#include <array>
#include <numeric>
#include <unordered_map>
#include <vector>

#include "point.hpp"

namespace strategy {

inline int get_life(const model::Unit&) {
    return 0;
}

inline int get_life(const model::LivingUnit& unit) {
    return unit.getLife();
}

inline int get_remaining_action_cooldown_ticks(const model::Unit&) {
    return 0;
}

inline int get_remaining_action_cooldown_ticks(const model::Building& unit) {
    return unit.getRemainingActionCooldownTicks();
}

inline int get_remaining_action_cooldown_ticks(const model::Minion& unit) {
    return unit.getRemainingActionCooldownTicks();
}

inline int get_remaining_action_cooldown_ticks(const model::Wizard& unit) {
    return unit.getRemainingActionCooldownTicks();
}

template <class T>
class CachedUnit {
public:
    static constexpr std::size_t LIFE_CHANGES_SIZE = 5;
    static constexpr std::size_t SPEEDS_SIZE = 10;

    using Value = T;

    CachedUnit() = default;

    CachedUnit(const Value& value, Tick tick)
        : value_(value),
          first_seen_(tick),
          last_seen_(tick),
          prev_life_change_(tick),
          prev_life_(get_life(value)),
          first_position_(value.getX(), value.getY()),
          last_idle_(get_remaining_action_cooldown_ticks(value) ? 0 : tick),
          last_action_(get_remaining_action_cooldown_ticks(value) ? tick : -1),
          last_move_(value.getSpeedX() || value.getSpeedY() ? tick : -1) {
        std::fill(speeds_.begin(), speeds_.end(), Point(value.getSpeedX(), value.getSpeedY()));
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
        const auto shift = std::min(speeds_.size(), std::size_t(last_seen - last_seen_));
        std::rotate(speeds_.rbegin(), speeds_.rbegin() + shift, speeds_.rend());
        std::fill_n(speeds_.begin() + 1, shift - 1, Point());
        speeds_.front() = Point(value.getSpeedX(), value.getSpeedY());
        last_seen_ = last_seen;
        if (const auto life_change = get_life(value) - prev_life_) {
            std::rotate(life_change_.rbegin(), life_change_.rbegin() + 1, life_change_.rend());
            life_change_.front() = {last_seen - prev_life_change_, life_change};
            prev_life_ = get_life(value);
            prev_life_change_ = last_seen;
        }
        if (get_remaining_action_cooldown_ticks(value)) {
            last_action_ = last_idle_ + 1;
        } else {
            last_idle_ = last_seen;
        }
        if (value.getSpeedX() || value.getSpeedY()) {
            last_move_ = last_seen;
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
        double weight = 1;
        double weights_sum = 0;
        Point result;

        for (const auto& speed : speeds_) {
            result = result + weight * speed;
            weights_sum += weight;
            weight *= 0.9;
        }

        return result / weights_sum;
    }

    Tick last_activity() const {
        return std::max(last_move_, last_action_);
    }

    bool is_active(Tick tick) const {
        return tick - last_activity() <= value().getCooldownTicks();
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
    Tick last_idle_ = 0;
    Tick last_action_ = -1;
    Tick last_move_ = -1;
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

using FullCache = std::tuple<
    Cache<model::Bonus>,
    Cache<model::Building>,
    Cache<model::Minion>,
    Cache<model::Projectile>,
    Cache<model::Tree>,
    Cache<model::Wizard>
>;

template <class T>
Cache<T>& get_cache(FullCache& cache) {
    return std::get<Cache<T>>(cache);
}

template <class T>
const Cache<T>& get_cache(const FullCache& cache) {
    return std::get<Cache<T>>(cache);
}

template <class T>
const typename Cache<T>::Units& get_units(const FullCache& cache) {
    return get_cache<T>(cache).units();
}

}
