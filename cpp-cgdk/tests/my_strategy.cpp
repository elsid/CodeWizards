#include "common.hpp"

#include <debug/output.hpp>
#include <MyStrategy.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace strategy {
namespace tests {

using namespace testing;

TEST(MyStrategy, simple) {
    const model::World world(
        0, // TickIndex
        20000, // TickCount
        4000, // Width
        4000, // Height
        {}, // Players
        {SELF}, // Wizards
        {}, // Minions
        {}, // Projectiles
        {}, // Bonuses
        {}, // Buildings
        {} // Trees
    );
    model::Move move;
    MyStrategy().move(SELF, world, GAME, move);
    EXPECT_EQ(move.getSpeed(), 4);
    EXPECT_EQ(move.getStrafeSpeed(), 0);
    EXPECT_EQ(move.getTurn(), 0);
}

}
}
