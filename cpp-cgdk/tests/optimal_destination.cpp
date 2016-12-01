#include "common.hpp"

#include <debug/output.hpp>
#include <optimal_destination.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace strategy {
namespace tests {

using namespace testing;

TEST(get_optimal_destination, for_all_default) {
    const model::World world;
    model::Move move;
    const Profiler profiler;
    const FullCache cache;
    const Context context(SELF, world, GAME, move, cache, profiler, Duration::max());
    WorldGraph graph(GAME);
    EXPECT_EQ(get_optimal_destination(context, graph, model::_LANE_UNKNOWN_), 44);
}

}
}
