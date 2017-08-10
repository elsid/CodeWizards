#include "common.hpp"

#include <debug/output.hpp>
#include <optimal_path.hpp>
#include <helpers.hpp>

#include <gtest/gtest.h>

namespace strategy {
namespace tests {

using namespace testing;

double length(const Path& path) {
    if (path.empty()) {
        return 0;
    }

    double result = 0;
    for (std::size_t i = 0; i < path.size() - 1; ++i) {
        result += path[i].distance(path[i + 1]);
    }
    return result;
}

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

TEST(get_tangent_points, all) {
    EXPECT_EQ(get_tangent_points(Circle(Point(10, 1), 1), Point(0, 0)),
              std::make_pair(Point(9.8019801980198018, 1.9801980198019802), Point(10, 0)));

    EXPECT_EQ(get_tangent_points(Circle(Point(10, -1), 1), Point(0, 0)),
              std::make_pair(Point(10, 0), Point(9.8019801980198018, -1.9801980198019802)));

    EXPECT_EQ(get_tangent_points(Circle(Point(1, 10), 1), Point(0, 0)),
              std::make_pair(Point(0, 10), Point(1.9801980198019802, 9.8019801980198018)));

    EXPECT_EQ(get_tangent_points(Circle(Point(-1, 10), 1), Point(0, 0)),
              std::make_pair(Point(-1.9801980198019802, 9.8019801980198018), Point(0, 10)));

    EXPECT_EQ(get_tangent_points(Circle(Point(110, 101), 1), Point(100, 100)),
              std::make_pair(Point(109.8019801980198018, 101.9801980198019802), Point(110, 100)));
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
    ASSERT_FALSE(result.empty());
    EXPECT_EQ(result.size(), 8u);
    EXPECT_EQ(result.front(), get_position(self));
    EXPECT_EQ(result.back(), target);
    EXPECT_NEAR(length(result), 298.412, 1e-3);
}

TEST(GetOptimalPath, with_static_barrier_at_direct_path_and_at_paths_around_it) {
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
    const model::Tree first(
        2, // Id
        1000 + 35 + 40 + 100, // X
        1000, // Y
        0, // SpeedX
        0, // SpeedY
        0, // Angle
        model::FACTION_OTHER, // Faction
        40, // Radius
        17, // Life
        17, // MaxLife
        {} // Statuses
    );
    const model::Tree second(
        3, // Id
        1000 + 35 + 40 + 100, // X
        1000 - 40 - 80 - 5, // Y
        0, // SpeedX
        0, // SpeedY
        0, // Angle
        model::FACTION_OTHER, // Faction
        80, // Radius
        17, // Life
        17, // MaxLife
        {} // Statuses
    );
    const model::Tree third(
        4, // Id
        1000 + 35 + 40 + 100, // X
        1000 + 40 + 80 + 5, // Y
        0, // SpeedX
        0, // SpeedY
        0, // Angle
        model::FACTION_OTHER, // Faction
        80, // Radius
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
        {first, second, third} // Trees
    );
    model::Move move;
    const Profiler profiler;
    FullCache cache;
    update_cache(cache, world);
    const Context context(SELF, world, GAME, move, cache, cache, profiler, Duration::max());
    const Point target(1300, 1000);
    const auto result = GetOptimalPath().step_size(3)(context, target);
    ASSERT_FALSE(result.empty());
    EXPECT_EQ(result.size(), 103u);
    EXPECT_EQ(result.front(), get_position(self));
    EXPECT_EQ(result.back(), target);
    EXPECT_NEAR(length(result), 1101.9328576726953, 1e-3);
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
    ASSERT_FALSE(result.empty());
    EXPECT_EQ(result.size(), 2u);
    EXPECT_EQ(result.front(), get_position(self));
    EXPECT_EQ(result.back(), target);
    EXPECT_NEAR(length(result), 282.843, 1e-3);
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
    ASSERT_FALSE(result.empty());
    EXPECT_EQ(result.size(), 2u);
    EXPECT_EQ(result.front(), get_position(self));
    EXPECT_EQ(result.back(), target);
    EXPECT_NEAR(length(result), 282.843, 1e-3);
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
    ASSERT_FALSE(result.empty());
    EXPECT_EQ(result.size(), 4u);
    EXPECT_EQ(result.front(), get_position(self));
    EXPECT_EQ(result.back(), target);
    EXPECT_NEAR(length(result), 349.772, 1e-3);
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
    ASSERT_FALSE(result.empty());
    EXPECT_EQ(result.size(), 2u);
    EXPECT_EQ(result.front(), get_position(self));
    EXPECT_EQ(result.back(), Point(1169.7105516901372, 1226.4491837508108));
    EXPECT_NEAR(length(result), 282.986, 1e-3);
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
    ASSERT_FALSE(result.empty());
    EXPECT_EQ(result.size(), 2u);
    EXPECT_EQ(result.front(), get_position(self));
    EXPECT_EQ(result.back(), Point(1169.7105516901372, 1226.4491837508108));
    EXPECT_NEAR(length(result), 282.986, 1e-3);
}

}
}
