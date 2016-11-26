#pragma once

#include "point.hpp"
#include "profiler.hpp"

#include "model/Game.h"
#include "model/Move.h"
#include "model/Wizard.h"
#include "model/World.h"

namespace strategy {

using UnitId = long long;

class Context {
public:
    Context(const model::Wizard& self, const model::World& world, const model::Game& game, model::Move& move,
            const Profiler& profiler, Duration time_limit)
        : self_(self), world_(world), game_(game), move_(move), profiler_(profiler), time_limit_(time_limit) {}

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
    const Profiler& profiler_;
    Duration time_limit_;
};

inline Point get_position(const model::Unit& unit) {
    return Point(unit.getX(), unit.getY());
}

inline Point get_speed(const model::Unit& unit) {
    return Point(unit.getSpeedX(), unit.getSpeedY());
}

}
