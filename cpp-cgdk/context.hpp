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

class Target {
public:
    Target() = default;

    Target(const model::Bonus* unit) : bonus_(unit) {}
    Target(const model::Building* unit) : building_(unit) {}
    Target(const model::Minion* unit) : minion_(unit) {}
    Target(const model::Wizard* unit) : wizard_(unit) {}
    Target(const model::Tree* unit) : tree_(unit) {}

    const model::Bonus* bonus() const { return bonus_; }
    const model::Building* building() const { return building_; }
    const model::Minion* minion() const { return minion_; }
    const model::Wizard* wizard() const { return wizard_; }
    const model::Tree* tree() const { return tree_; }

    bool has_value() const {
        return bonus_ || building_ || minion_ || wizard_ || tree_;
    }

    const model::CircularUnit* unit() const {
        if (const auto unit = bonus_) {
            return unit;
        } else if (const auto unit = building_) {
            return unit;
        } else if (const auto unit = minion_) {
            return unit;
        } else if (const auto unit = wizard_) {
            return unit;
        } else if (const auto unit = tree_) {
            return unit;
        }
        return nullptr;
    }

private:
    const model::Bonus* bonus_ = nullptr;
    const model::Building* building_ = nullptr;
    const model::Minion* minion_ = nullptr;
    const model::Wizard* wizard_ = nullptr;
    const model::Tree* tree_ = nullptr;
};

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

template <class T>
const typename Cache<T>::Units& get_units(const FullCache& cache) {
    return std::get<T>(cache).units();
}

}
