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

class Context {
public:
    Context(const model::Wizard& self, const model::World& world, const model::Game& game, model::Move& move,
            const FullCache& cache, const Profiler& profiler, Duration time_limit)
        : self_(self), world_(world), game_(game), move_(move),
          cache_(cache), profiler_(profiler), time_limit_(time_limit) {}

    const model::Wizard& self() const {
        return self_;
    }

    const model::World& world() const {
        return world_;
    }

    const model::Game& game() const {
        return game_;
    }

    model::Move& move() {
        return move_;
    }

    const FullCache& cache() const {
        return cache_;
    }

    Duration time_left() const {
        return time_limit_ - profiler_.duration();
    }

    const Profiler& profiler() const {
        return profiler_;
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

}
