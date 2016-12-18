#include "common.hpp"

#include <debug/output.hpp>
#include <optimal_destination.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace strategy {
namespace tests {

using namespace testing;

TEST(get_optimal_destination, for_all_default) {
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
    const Profiler profiler;
    FullCache cache;
    update_cache(cache, world);
    const Context context(SELF, world, GAME, move, cache, cache, profiler, Duration::max());
    WorldGraph graph(GAME);
    EXPECT_EQ(get_optimal_destination(context, graph, model::_LANE_UNKNOWN_, SELF), 46u);
}

}
}
