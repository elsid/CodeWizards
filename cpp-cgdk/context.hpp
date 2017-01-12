#pragma once

#include "point.hpp"
#include "profiler.hpp"
#include "cache.hpp"
#include "common.hpp"

#include "model/Game.h"
#include "model/Move.h"
#include "model/Wizard.h"
#include "model/World.h"

#include <sstream>

namespace strategy {

struct Timeout : std::runtime_error {
    using std::runtime_error::runtime_error;
};

class Context {
public:
    Context(const model::Wizard& self, const model::World& world, const model::Game& game, model::Move& move,
            const FullCache& cache, const FullCache& history_cache, const Profiler& profiler, Duration time_limit)
        : self_(self), world_(world), game_(game), move_(move),
          cache_(cache), history_cache_(history_cache), profiler_(profiler), time_limit_(time_limit),
          cached_self_(get_units<model::Wizard>(cache).at(self.getId())) {}

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

    const FullCache& history_cache() const {
        return history_cache_;
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

    Duration time_left() const {
        return time_limit() - profiler().duration();
    }

    const CachedUnit<model::Wizard>& cached_self() const {
        return cached_self_;
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
    const FullCache& history_cache_;
    const Profiler& profiler_;
    Duration time_limit_;
    const CachedUnit<model::Wizard>& cached_self_;
};

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
