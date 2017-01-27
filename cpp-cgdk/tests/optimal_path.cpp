#include "common.hpp"

#include <debug/output.hpp>
#include <optimal_path.hpp>
#include <helpers.hpp>

#include <gtest/gtest.h>

namespace strategy {
namespace tests {

using namespace testing;

TEST(has_intersection_with_borders, all) {
    EXPECT_TRUE(has_intersection_with_borders(Circle(Point(0, 0), 10), 100));
    EXPECT_TRUE(has_intersection_with_borders(Circle(Point(50, 0), 10), 100));
    EXPECT_TRUE(has_intersection_with_borders(Circle(Point(100, 0), 10), 100));

    EXPECT_TRUE(has_intersection_with_borders(Circle(Point(0, 50), 10), 100));
    EXPECT_TRUE(has_intersection_with_borders(Circle(Point(100, 50), 10), 100));

    EXPECT_TRUE(has_intersection_with_borders(Circle(Point(0, 100), 10), 100));
    EXPECT_TRUE(has_intersection_with_borders(Circle(Point(50, 100), 10), 100));
    EXPECT_TRUE(has_intersection_with_borders(Circle(Point(100, 100), 10), 100));

    EXPECT_TRUE(has_intersection_with_borders(Circle(Point(10, 10), 10), 100));
    EXPECT_TRUE(has_intersection_with_borders(Circle(Point(50, 10), 10), 100));
    EXPECT_TRUE(has_intersection_with_borders(Circle(Point(90, 10), 10), 100));

    EXPECT_TRUE(has_intersection_with_borders(Circle(Point(10, 50), 10), 100));
    EXPECT_TRUE(has_intersection_with_borders(Circle(Point(90, 50), 10), 100));

    EXPECT_TRUE(has_intersection_with_borders(Circle(Point(10, 90), 10), 100));
    EXPECT_TRUE(has_intersection_with_borders(Circle(Point(50, 90), 10), 100));
    EXPECT_TRUE(has_intersection_with_borders(Circle(Point(90, 90), 10), 100));

    EXPECT_FALSE(has_intersection_with_borders(Circle(Point(10 + 2e-8, 10 + 2e-8), 10), 100));
    EXPECT_FALSE(has_intersection_with_borders(Circle(Point(50, 10 + 2e-8), 10), 100));
    EXPECT_FALSE(has_intersection_with_borders(Circle(Point(90 - 2e-8, 10 + 2e-8), 10), 100));

    EXPECT_FALSE(has_intersection_with_borders(Circle(Point(10 + 2e-8, 50), 10), 100));
    EXPECT_FALSE(has_intersection_with_borders(Circle(Point(50, 50), 10), 100));
    EXPECT_FALSE(has_intersection_with_borders(Circle(Point(90 - 2e-8, 50), 10), 100));

    EXPECT_FALSE(has_intersection_with_borders(Circle(Point(10 + 2e-8, 90 - 2e-8), 10), 100));
    EXPECT_FALSE(has_intersection_with_borders(Circle(Point(50, 90 - 2e-8), 10), 100));
    EXPECT_FALSE(has_intersection_with_borders(Circle(Point(90 - 2e-8, 90 - 2e-8), 10), 100));
}

TEST(GetOptimalPath, with_only_me) {
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
    const Point target(1200, 1200);
    const auto result = GetOptimalPath().step_size(3)(context, target);
    EXPECT_EQ(result, Path({get_position(SELF), target}));
}

TEST(GetOptimalPath, with_only_me_with_shift) {
    const model::Wizard self(
        1, // Id
        1000.5, // X
        1000.5, // Y
        0, // SpeedX
        0, // SpeedY
        0, // Angle
        model::FACTION_ACADEMY, // Faction
        35, // Radius
        100, // Life
        100, // MaxLife
        {}, // Statuses
        1, // OwnerPlayerId
        true, // Me
        100, // Mana
        100, // MaxMana
        600, // VisionRange
        500, // CastRange
        0, // Xp
        0, // Level
        {}, // Skills
        0, // RemainingActionCooldownTicks
        {0, 0, 0, 0, 0, 0, 0}, // RemainingCooldownTicksByAction
        true, // Master
        {} // Messages
    );
    const model::World world(
        0, // TickIndex
        20000, // TickCount
        4000, // Width
        4000, // Height
        {}, // Players
        {self}, // Wizards
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
    const Context context(self, world, GAME,move, cache, cache, profiler, Duration::max());
    const Point target(1200.3, 1200.3);
    const auto result = GetOptimalPath().step_size(3)(context, target);
    EXPECT_EQ(result, Path({get_position(self), target}));
}

TEST(GetOptimalPath, with_only_me_from_position_near_border) {
    const model::Wizard self(
        1, // Id
        35.01, // X
        35.01, // Y
        0, // SpeedX
        0, // SpeedY
        0, // Angle
        model::FACTION_ACADEMY, // Faction
        35, // Radius
        100, // Life
        100, // MaxLife
        {}, // Statuses
        1, // OwnerPlayerId
        true, // Me
        100, // Mana
        100, // MaxMana
        600, // VisionRange
        500, // CastRange
        0, // Xp
        0, // Level
        {}, // Skills
        0, // RemainingActionCooldownTicks
        {0, 0, 0, 0, 0, 0, 0}, // RemainingCooldownTicksByAction
        true, // Master
        {} // Messages
    );
    const model::World world(
        0, // TickIndex
        20000, // TickCount
        4000, // Width
        4000, // Height
        {}, // Players
        {self}, // Wizards
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
    const Context context(self, world, GAME,move, cache, cache, profiler, Duration::max());
    const Point target(1000, 1000);
    const auto result = GetOptimalPath().step_size(3)(context, target);
    EXPECT_EQ(result, Path({get_position(self), target}));
}

TEST(GetOptimalPath, with_only_me_to_my_position) {
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
    const Context context(SELF, world, GAME,move, cache, cache, profiler, Duration::max());
    const auto result = GetOptimalPath().step_size(3)(context, get_position(SELF));
    EXPECT_EQ(result, Path({get_position(SELF)}));
}

TEST(GetOptimalPath, with_static_barrier) {
    const model::Wizard self(
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
        true, // Me
        100, // Mana
        100, // MaxMana
        600, // VisionRange
        500, // CastRange
        0, // Xp
        0, // Level
        {}, // Skills
        0, // RemainingActionCooldownTicks
        {0, 0, 0, 0, 0, 0, 0}, // RemainingCooldownTicksByAction
        true, // Master
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
        {self}, // Wizards
        {}, // Minions
        {}, // Projectiles
        {}, // Bonuses
        {}, // Buildings
        {tree} // Trees
    );
    model::Move move;
    const Profiler profiler;
    FullCache cache;
    update_cache(cache, world);
    const Context context(SELF, world, GAME,move, cache, cache, profiler, Duration::max());
    const Point target(1200, 1200);
    const auto result = GetOptimalPath().step_size(3)(context, target);
    ASSERT_EQ(result.size(), 29u);
    EXPECT_EQ(result.front(), get_position(self));
    EXPECT_EQ(result.back(), target);
}

TEST(GetOptimalPath, with_dynamic_barrier_moving_in_same_direction) {
    const model::Wizard self(
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
        true, // Me
        100, // Mana
        100, // MaxMana
        600, // VisionRange
        500, // CastRange
        0, // Xp
        0, // Level
        {}, // Skills
        0, // RemainingActionCooldownTicks
        {0, 0, 0, 0, 0, 0, 0}, // RemainingCooldownTicksByAction
        true, // Master
        {} // Messages
    );
    const model::Wizard other(
        2, // Id
        1000 + self.getRadius() + GAME.getWizardRadius() + 10, // X
        1000 + self.getRadius() + GAME.getWizardRadius() + 10, // Y
        3, // SpeedX
        3, // SpeedY
        0, // Angle
        model::FACTION_ACADEMY, // Faction
        35, // Radius
        100, // Life
        100, // MaxLife
        {}, // Statuses
        1, // OwnerPlayerId
        false, // Me
        100, // Mana
        100, // MaxMana
        600, // VisionRange
        500, // CastRange
        0, // Xp
        0, // Level
        {}, // Skills
        0, // RemainingActionCooldownTicks
        {0, 0, 0, 0, 0, 0, 0}, // RemainingCooldownTicksByAction
        false, // Master
        {} // Messages
    );
    const model::World world(
        0, // TickIndex
        20000, // TickCount
        4000, // Width
        4000, // Height
        {}, // Players
        {self, other}, // Wizards
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
    const Context context(SELF, world, GAME,move, cache, cache, profiler, Duration::max());
    const Point target(1200, 1200);
    const auto result = GetOptimalPath().step_size(3)(context, target);
    ASSERT_EQ(result.size(), 33u);
    EXPECT_EQ(result.front(), get_position(self));
    EXPECT_EQ(result.back(), target);
}

TEST(GetOptimalPath, with_dynamic_barrier_moving_in_opposite_direction) {
    const model::Wizard self(
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
        true, // Me
        100, // Mana
        100, // MaxMana
        600, // VisionRange
        500, // CastRange
        0, // Xp
        0, // Level
        {}, // Skills
        0, // RemainingActionCooldownTicks
        {0, 0, 0, 0, 0, 0, 0}, // RemainingCooldownTicksByAction
        true, // Master
        {} // Messages
    );
    const model::Wizard other(
        2, // Id
        1000 + self.getRadius() + GAME.getWizardRadius() + 10, // X
        1000 + self.getRadius() + GAME.getWizardRadius() + 10, // Y
        -3, // SpeedX
        -3, // SpeedY
        0, // Angle
        model::FACTION_ACADEMY, // Faction
        35, // Radius
        100, // Life
        100, // MaxLife
        {}, // Statuses
        1, // OwnerPlayerId
        false, // Me
        100, // Mana
        100, // MaxMana
        600, // VisionRange
        500, // CastRange
        0, // Xp
        0, // Level
        {}, // Skills
        0, // RemainingActionCooldownTicks
        {0, 0, 0, 0, 0, 0, 0}, // RemainingCooldownTicksByAction
        false, // Master
        {} // Messages
    );
    const model::World world(
        0, // TickIndex
        20000, // TickCount
        4000, // Width
        4000, // Height
        {}, // Players
        {self, other}, // Wizards
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
    const Context context(SELF, world, GAME,move, cache, cache, profiler, Duration::max());
    const Point target(1200, 1200);
    const auto result = GetOptimalPath().step_size(3)(context, target);
    ASSERT_EQ(result.size(), 4u);
    EXPECT_EQ(result.front(), get_position(self));
    EXPECT_EQ(result.back(), target);
}

TEST(GetOptimalPath, with_dynamic_barrier_moving_in_crossing_direction) {
    const model::Wizard self(
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
        true, // Me
        100, // Mana
        100, // MaxMana
        600, // VisionRange
        500, // CastRange
        0, // Xp
        0, // Level
        {}, // Skills
        0, // RemainingActionCooldownTicks
        {0, 0, 0, 0, 0, 0, 0}, // RemainingCooldownTicksByAction
        true, // Master
        {} // Messages
    );
    const model::Wizard other(
        2, // Id
        1000 + self.getRadius() + GAME.getWizardRadius() + 10, // X
        1000, // Y
        -3, // SpeedX
        3, // SpeedY
        0, // Angle
        model::FACTION_ACADEMY, // Faction
        35, // Radius
        100, // Life
        100, // MaxLife
        {}, // Statuses
        1, // OwnerPlayerId
        false, // Me
        100, // Mana
        100, // MaxMana
        600, // VisionRange
        500, // CastRange
        0, // Xp
        0, // Level
        {}, // Skills
        0, // RemainingActionCooldownTicks
        {0, 0, 0, 0, 0, 0, 0}, // RemainingCooldownTicksByAction
        false, // Master
        {} // Messages
    );
    const model::World world(
        0, // TickIndex
        20000, // TickCount
        4000, // Width
        4000, // Height
        {}, // Players
        {self, other}, // Wizards
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
    const Context context(SELF, world, GAME,move, cache, cache, profiler, Duration::max());
    const Point target(1200, 1200);
    const auto result = GetOptimalPath().step_size(3)(context, target);
    ASSERT_EQ(result.size(), 11u);
    EXPECT_EQ(result.front(), get_position(self));
    EXPECT_EQ(result.back(), target);
}

TEST(GetOptimalPath, with_static_occupier) {
    const model::Wizard self(
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
        true, // Me
        100, // Mana
        100, // MaxMana
        600, // VisionRange
        500, // CastRange
        0, // Xp
        0, // Level
        {}, // Skills
        0, // RemainingActionCooldownTicks
        {0, 0, 0, 0, 0, 0, 0}, // RemainingCooldownTicksByAction
        true, // Master
        {} // Messages
    );
    const model::Tree tree(
        2, // Id
        1200, // X
        1200, // Y
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
        {self}, // Wizards
        {}, // Minions
        {}, // Projectiles
        {}, // Bonuses
        {}, // Buildings
        {tree} // Trees
    );
    model::Move move;
    const Profiler profiler;
    FullCache cache;
    update_cache(cache, world);
    const Context context(SELF, world, GAME,move, cache, cache, profiler, Duration::max());
    const Point target(1200, 1200);
    const auto result = GetOptimalPath().step_size(3)(context, target);
    ASSERT_EQ(result.size(), 61u);
    EXPECT_EQ(result.front(), get_position(self));
    EXPECT_EQ(result.back(), target - Point(20, 35));
}

TEST(GetOptimalPath, with_static_occupier_and_limited_iterations) {
    const model::Wizard self(
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
        true, // Me
        100, // Mana
        100, // MaxMana
        600, // VisionRange
        500, // CastRange
        0, // Xp
        0, // Level
        {}, // Skills
        0, // RemainingActionCooldownTicks
        {0, 0, 0, 0, 0, 0, 0}, // RemainingCooldownTicksByAction
        true, // Master
        {} // Messages
    );
    const model::Tree tree(
        2, // Id
        1200, // X
        1200, // Y
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
        {self}, // Wizards
        {}, // Minions
        {}, // Projectiles
        {}, // Bonuses
        {}, // Buildings
        {tree} // Trees
    );
    model::Move move;
    const Profiler profiler;
    FullCache cache;
    update_cache(cache, world);
    const Context context(SELF, world, GAME,move, cache, cache, profiler, Duration::max());
    const Point target(1200, 1200);
    const auto result = GetOptimalPath().step_size(3).max_iterations(50)(context, target);
    ASSERT_EQ(result.size(), 50u);
    EXPECT_EQ(result.front(), get_position(self));
    EXPECT_EQ(result.back(), target - Point(53, 56));
}

}
}
