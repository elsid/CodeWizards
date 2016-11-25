#pragma once

#include "point.hpp"

#include "model/Game.h"
#include "model/Move.h"
#include "model/Wizard.h"
#include "model/World.h"

namespace strategy {

using UnitId = long long;

struct Context {
    const model::Wizard& self;
    const model::World& world;
    const model::Game& game;
    model::Move& move;
};

inline Point get_position(const model::Unit& unit) {
    return Point(unit.getX(), unit.getY());
}

inline Point get_speed(const model::Unit& unit) {
    return Point(unit.getSpeedX(), unit.getSpeedY());
}

}
