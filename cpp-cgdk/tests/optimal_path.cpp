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
        {SELF}, // Wizards
        {}, // Minions
        {}, // Projectiles
        {}, // Bonuses
        {}, // Buildings
        {} // Trees
    );
    model::Move move;
    const Profiler profiler;
    const FullCache cache;
    const Context context(SELF, world, GAME, move, cache, profiler, Duration::max());
    const Point target(1200, 1200);
    const auto step_size = 3;
    const auto result = get_optimal_path(context, target, step_size);
    EXPECT_EQ(result, Path({get_position(SELF), target}));
}

TEST(get_optimal_path, with_only_me_with_shift) {
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
    const FullCache cache;
    const Context context(self, world, GAME, move, cache, profiler, Duration::max());
    const Point target(1200.3, 1200.3);
    const auto step_size = 3;
    const auto result = get_optimal_path(context, target, step_size);
    EXPECT_EQ(result, Path({get_position(self), target}));
}

TEST(get_optimal_path, with_only_me_to_my_position) {
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
    const FullCache cache;
    const Context context(SELF, world, GAME, move, cache, profiler, Duration::max());
    const auto step_size = 3;
    const auto result = get_optimal_path(context, get_position(SELF), step_size);
    EXPECT_EQ(result, Path({get_position(SELF)}));
}

TEST(get_optimal_path, with_static_barrier) {
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
    const FullCache cache;
    const Context context(SELF, world, GAME, move, cache, profiler, Duration::max());
    const Point target(1200, 1200);
    const auto step_size = 3;
    const auto result = get_optimal_path(context, target, step_size);
    ASSERT_EQ(result.size(), 29);
    EXPECT_EQ(result.front(), get_position(self));
    EXPECT_EQ(result.back(), target);
}

TEST(get_optimal_path, with_dynamic_barrier_moving_in_same_direction) {
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
    const FullCache cache;
    const Context context(SELF, world, GAME, move, cache, profiler, Duration::max());
    const Point target(1200, 1200);
    const auto step_size = 3;
    const auto result = get_optimal_path(context, target, step_size);
    ASSERT_EQ(result.size(), 47);
    EXPECT_EQ(result.front(), get_position(self));
    EXPECT_EQ(result.back(), target);
}

TEST(get_optimal_path, with_dynamic_barrier_moving_in_opposite_direction) {
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
    const FullCache cache;
    const Context context(SELF, world, GAME, move, cache, profiler, Duration::max());
    const Point target(1200, 1200);
    const auto step_size = 3;
    const auto result = get_optimal_path(context, target, step_size);
    ASSERT_EQ(result.size(), 7);
    EXPECT_EQ(result.front(), get_position(self));
    EXPECT_EQ(result.back(), target);
}

TEST(get_optimal_path, with_dynamic_barrier_moving_in_crossing_direction) {
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
    const FullCache cache;
    const Context context(SELF, world, GAME, move, cache, profiler, Duration::max());
    const Point target(1200, 1200);
    const auto step_size = 3;
    const auto result = get_optimal_path(context, target, step_size);
    ASSERT_EQ(result.size(), 13);
    EXPECT_EQ(result.front(), get_position(self));
    EXPECT_EQ(result.back(), target);
}

TEST(get_optimal_path, with_static_occupier) {
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
    const FullCache cache;
    const Context context(SELF, world, GAME, move, cache, profiler, Duration::max());
    const Point target(1200, 1200);
    const auto step_size = 3;
    const auto result = get_optimal_path(context, target, step_size);
    ASSERT_EQ(result.size(), 58);
    EXPECT_EQ(result.front(), get_position(self));
    EXPECT_EQ(result.back(), target - Point(29, 29));
}

}
}
