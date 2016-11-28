#pragma once

#include "point.hpp"
#include "profiler.hpp"
#include "cache.hpp"
#include "common.hpp"

#include "model/Game.h"
#include "model/Move.h"
#include "model/Wizard.h"
#include "model/World.h"

namespace strategy {

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

template <class T>
typename Cache<T>::Units::const_iterator find_unit(const FullCache& cache, Id<T> id) {
    const auto& units = get_units<T>(cache);
    return std::find_if(units.begin(), units.end(), [&] (const auto& unit) { return unit.first == id.value(); });
}

template <class T>
bool is_end(const FullCache& cache, const typename Cache<T>::Units::const_iterator it) {
    return get_units<T>(cache).end() == it;
}

template <class T>
Id<T> get_id(const T& unit) {
    return Id<T>(unit.getId());
}

class Target {
public:
    template <class T>
    using Pair = std::pair<bool, Id<T>>;

    Target() = default;

    template <class T>
    Target(Id<T> id) {
        std::get<Pair<T>>(ids_) = {true, id};
    }

    template <class T>
    Id<T> id() const {
        return std::get<Pair<T>>(ids_).second;
    }

    template <class T>
    bool is() const {
        return std::get<Pair<T>>(ids_).first;
    }

    bool is_some() const {
        return is<model::Bonus>() || is<model::Building>() || is<model::Minion>() || is<model::Wizard>() || is<model::Tree>();
    }

    template <class T>
    const CachedUnit<T>* cached_unit(const FullCache& cache) const {
        if (!is<T>()) {
            return nullptr;
        }
        const auto it = find_unit(cache, id<T>());
        return is_end<T>(cache, it) ? nullptr : &it->second;
    }

    template <class T>
    const T* unit(const FullCache& cache) const {
        const auto cached = cached_unit<T>(cache);
        return cached ? &cached->value() : nullptr;
    }

    const model::CircularUnit* circular_unit(const FullCache& cache) const {
        if (is<model::Bonus>()) {
            return unit<model::Bonus>(cache);
        } else if (is<model::Building>()) {
            return unit<model::Building>(cache);
        } else if (is<model::Minion>()) {
            return unit<model::Minion>(cache);
        } else if (is<model::Wizard>()) {
            return unit<model::Wizard>(cache);
        } else if (is<model::Tree>()) {
            return unit<model::Tree>(cache);
        }
        return nullptr;
    }

private:
    std::tuple<
        std::pair<bool, Id<model::Bonus>>,
        std::pair<bool, Id<model::Building>>,
        std::pair<bool, Id<model::Minion>>,
        std::pair<bool, Id<model::Wizard>>,
        std::pair<bool, Id<model::Tree>>
    > ids_;
};

struct Timeout : std::runtime_error {
    using std::runtime_error::runtime_error;
};

class Context {
public:
    Context(const model::Wizard& self, const model::World& world, const model::Game& game, model::Move& move,
            const FullCache& cache, const Profiler& profiler, Duration time_limit)
        : self_(self), world_(world), game_(game), move_(move),
          cache_(cache), profiler_(profiler), time_limit_(time_limit) {}

    Context(const Context&) = delete;
    Context(Context&&) = delete;

    const model::Wizard& self() const {
        return self_;
    }

    const model::World& world() const {
        return world_;
    }

    const model::Game& game() const {
        return game_;
    }

    const model::Move& move() const {
        return move_;
    }

    model::Move& move() {
        return move_;
    }

    const FullCache& cache() const {
        return cache_;
    }

    const Profiler& profiler() const {
        return profiler_;
    }

    Duration time_limit() const {
        return time_limit_;
    }

    void time_limit(Duration value) {
        time_limit_ = value;
    }

    void check_timeout(const char* function, const char* file, int line) const {
        using Ms = std::chrono::duration<double, std::milli>;
        if (profiler().duration() > time_limit()) {
            std::ostringstream message;
            message << "Timeout in " << function << " (" << file << ":" << line << "): limit " << Ms(time_limit()).count()
                    << "ms, elapsed " << Ms(profiler_.duration()).count() << "ms";
            throw Timeout(message.str());
        }
    }

private:
    const model::Wizard& self_;
    const model::World& world_;
    const model::Game& game_;
    model::Move& move_;
    const FullCache& cache_;
    const Profiler& profiler_;
    Duration time_limit_;
};

inline Point get_position(const model::Unit& unit) {
    return Point(unit.getX(), unit.getY());
}

inline Point get_speed(const model::Unit& unit) {
    return Point(unit.getSpeedX(), unit.getSpeedY());
}

template <class T>
const std::vector<T>& get_units(const model::World& world);

template <>
inline const std::vector<model::Bonus>& get_units(const model::World& world) {
    return world.getBonuses();
}

template <>
inline const std::vector<model::Building>& get_units(const model::World& world) {
    return world.getBuildings();
}

template <>
inline const std::vector<model::Minion>& get_units(const model::World& world) {
    return world.getMinions();
}

template <>
inline const std::vector<model::Projectile>& get_units(const model::World& world) {
    return world.getProjectiles();
}

template <>
inline const std::vector<model::Tree>& get_units(const model::World& world) {
    return world.getTrees();
}

template <>
inline const std::vector<model::Wizard>& get_units(const model::World& world) {
    return world.getWizards();
}

template <class T>
void update_specific_cache(FullCache& cache, const model::World& world) {
    std::get<Cache<T>>(cache).update(get_units<T>(world), world.getTickIndex());
}

inline void update_cache(FullCache& cache, const model::World& world) {
    update_specific_cache<model::Bonus>(cache, world);
    update_specific_cache<model::Building>(cache, world);
    update_specific_cache<model::Minion>(cache, world);
    update_specific_cache<model::Projectile>(cache, world);
    update_specific_cache<model::Tree>(cache, world);
    update_specific_cache<model::Wizard>(cache, world);
}

template <class T, class Predicate>
void invalidate_specific_cache(FullCache& cache, const Predicate& predicate) {
    std::get<Cache<T>>(cache).invalidate(predicate);
}

template <class Predicate>
inline void invalidate_cache(FullCache& cache, const Predicate& predicate) {
    invalidate_specific_cache<model::Bonus>(cache, predicate);
    invalidate_specific_cache<model::Building>(cache, predicate);
    invalidate_specific_cache<model::Minion>(cache, predicate);
    invalidate_specific_cache<model::Projectile>(cache, predicate);
    invalidate_specific_cache<model::Tree>(cache, predicate);
    invalidate_specific_cache<model::Wizard>(cache, predicate);
}

}
