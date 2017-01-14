#pragma once

#include "minion_move.hpp"

#include <model/Game.h>
#include <model/World.h>

namespace strategy {
namespace simulation {

class MinionStrategy {
public:
    void move(const model::Minion& self, const model::World& world, const model::Game& game, MinionMove& move);
};

} // namespace simulation
} // namespace strategy
