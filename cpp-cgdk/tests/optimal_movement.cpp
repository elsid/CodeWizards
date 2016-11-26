#include "common.hpp"

#include <debug/output.hpp>
#include <optimal_movement.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace strategy {

bool operator ==(const MovementState& lhs, const MovementState& rhs) {
    return lhs.tick() == rhs.tick() && lhs.position() == rhs.position() && lhs.angle() == rhs.angle();
}

namespace tests {

using namespace testing;

TEST(get_optimal_movement, only_for_me) {
    const model::World world(
        0, // TickIndex
        20000, // TickCount
        4000, // Width
        4000, // Height
        {}, // Players
        {WIZARD}, // Wizards
        {}, // Minions
        {}, // Projectiles
        {}, // Bonuses
        {}, // Buildings
        {} // Trees
    );
    model::Move move;
    const Context context {WIZARD, world, GAME, move};
    const Path path({get_position(WIZARD), Point(2000, 2000)});
    const OptPoint look_target;
    const auto result = get_optimal_movement(context, path, look_target);
    ASSERT_EQ(result.first.size(), 617);
    ASSERT_EQ(result.second.size(), 616);
    EXPECT_EQ(result.first.back(), MovementState(616, Point(1998.3310126883935, 1997.6931826193488), 1.6756669282041097));
}

}
}
