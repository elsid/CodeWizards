#include "common.hpp"

#include <debug/output.hpp>
#include <optimal_path.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace strategy {
namespace tests {

using namespace testing;

TEST(get_optimal_path, with_only_me) {
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
    const Point target(2000, 2000);
    const auto step_size = 20;
    const auto result = get_optimal_path(context, target, step_size);
    ASSERT_FALSE(result.empty());
    EXPECT_EQ(result.back(), target);
}

}
}
