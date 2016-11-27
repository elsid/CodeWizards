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
    const Profiler profiler;
    const FullCache cache;
    const Context context(WIZARD, world, GAME, move, cache, profiler, Duration::max());
    const Point target(1200, 1200);
    const auto step_size = 3;
    const auto result = get_optimal_path(context, target, step_size);
    ASSERT_FALSE(result.empty());
    EXPECT_EQ(result.back(), target);
    EXPECT_EQ(result.size(), 2);
}

TEST(get_optimal_path, with_static_barriers) {
    const model::Wizard wizard(
        1, // Id
        1000, // X
        1000, // Y
        0, // SpeedX
        0, // SpeedY
        0, // Angle
        model::FACTION_ACADEMY, // Faction
        35, // Radius
        100, // Life
        100, // MaxLife
        {}, // Statuses
        1, // OwnerPlayerId
        1, // Me
        100, // Mana
        100, // MaxMana
        600, // VisionRange
        500, // CastRange
        0, // Xp
        0, // Level
        {}, // Skills
        0, // RemainingActionCooldownTicks
        {0, 0, 0, 0, 0, 0, 0}, // RemainingCooldownTicksByAction
        1, // Master
        {} // Messages
    );
    const model::Tree tree(
        2, // Id
        1000 + 35 + 5 + 10, // X
        1000 + 35 + 5 + 10, // Y
        0, // SpeedX
        0, // SpeedY
        0, // Angle
        model::FACTION_OTHER, // Faction
        5, // Radius
        17, // Life
        17, // MaxLife
        {} // Statuses
    );
    const model::World world(
        0, // TickIndex
        20000, // TickCount
        4000, // Width
        4000, // Height
        {}, // Players
        {wizard}, // Wizards
        {}, // Minions
        {}, // Projectiles
        {}, // Bonuses
        {}, // Buildings
        {tree} // Trees
    );
    model::Move move;
    const Profiler profiler;
    const FullCache cache;
    const Context context(WIZARD, world, GAME, move, cache, profiler, Duration::max());
    const Point target(1200, 1200);
    const auto step_size = 3;
    const auto result = get_optimal_path(context, target, step_size);
    ASSERT_FALSE(result.empty());
    EXPECT_EQ(result.back(), target);
    EXPECT_EQ(result.size(), 29);
}

}
}
