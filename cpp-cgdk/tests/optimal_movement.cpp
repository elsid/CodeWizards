#include "common.hpp"

#include <debug/output.hpp>
#include <optimal_movement.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace strategy {

bool operator ==(const Movement& lhs, const Movement& rhs) {
    return lhs.speed() == rhs.speed() && lhs.strafe_speed() == rhs.strafe_speed() && lhs.turn() == rhs.turn();
}

bool operator ==(const MovementState& lhs, const MovementState& rhs) {
    return lhs.tick() == rhs.tick() && lhs.position() == rhs.position() && lhs.angle() == rhs.angle();
}

namespace tests {

using namespace testing;

TEST(get_shift, all) {
    EXPECT_EQ(get_shift(MovementState(0, Point(), 0), Movement(0, 0, 0)), Point());
    EXPECT_EQ(get_shift(MovementState(0, Point(), 0), Movement(1, 0, 0)), Point(1, 0));
    EXPECT_EQ(get_shift(MovementState(0, Point(), 0), Movement(0, 1, 0)), Point(0, 1));
    EXPECT_EQ(get_shift(MovementState(0, Point(), 0), Movement(-1, 0, 0)), Point(-1, 0));
    EXPECT_EQ(get_shift(MovementState(0, Point(), 0), Movement(0, -1, 0)), Point(0, -1));
    EXPECT_EQ(get_shift(MovementState(0, Point(), 0), Movement(0, 0, 1)), Point());
    EXPECT_EQ(get_shift(MovementState(0, Point(), M_PI / 2), Movement(1, 0, 0)), Point(6.123233995736766e-17, 1));
    EXPECT_EQ(get_shift(MovementState(0, Point(), M_PI / 2), Movement(0, 1, 0)), Point(-1, 6.123233995736766e-17));
    EXPECT_EQ(get_shift(MovementState(0, Point(), M_PI / 2), Movement(-1, 0, 0)), Point(-6.123233995736766e-17, -1));
    EXPECT_EQ(get_shift(MovementState(0, Point(), M_PI / 2), Movement(0, -1, 0)), Point(1, -6.123233995736766e-17));
}

TEST(normalize_angle, all) {
    EXPECT_DOUBLE_EQ(normalize_angle(0.3 * M_PI), 0.3 * M_PI);
    EXPECT_DOUBLE_EQ(normalize_angle(M_PI), M_PI);
    EXPECT_DOUBLE_EQ(normalize_angle(-M_PI), -M_PI);
    EXPECT_DOUBLE_EQ(normalize_angle(2 * M_PI), 0);
    EXPECT_DOUBLE_EQ(normalize_angle(3 * M_PI), -M_PI);
    EXPECT_DOUBLE_EQ(normalize_angle(-3 * M_PI), M_PI);
}

TEST(get_next_movement, all) {
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
    EXPECT_EQ(get_next_movement(Point(), MovementState(0, Point(), 0), OptPoint(), Bounds(context)), Movement(0, 0, 0));
    EXPECT_EQ(get_next_movement(Point(1, 0), MovementState(0, Point(), 0), OptPoint(), Bounds(context)), Movement(1, 0, 0));
    EXPECT_EQ(get_next_movement(Point(0, 1), MovementState(0, Point(), 0), OptPoint(), Bounds(context)), Movement(8.1643119943156876e-17, 1, 0.10471999999999999));
    EXPECT_EQ(get_next_movement(Point(1, 0), MovementState(0, Point(), M_PI / 2), OptPoint(), Bounds(context)), Movement(8.1643119943156876e-17, -1, -0.10471999999999999));
    EXPECT_EQ(get_next_movement(Point(-1, 0), MovementState(0, Point(), 0), OptPoint(), Bounds(context)), Movement(-1, 1.224646799147353e-16, 0.10471999999999999));
    EXPECT_EQ(get_next_movement(Point(0, -1), MovementState(0, Point(), 0), OptPoint(), Bounds(context)), Movement(8.1643119943156876e-17, -1, -0.10471999999999999));
    EXPECT_EQ(get_next_movement(Point(0, -1), MovementState(0, Point(), M_PI / 2), OptPoint(), Bounds(context)), Movement(-1, -1.224646799147353e-16, -0.10471999999999999));
    EXPECT_EQ(get_next_movement(Point(1, 1), MovementState(0, Point(), 0), OptPoint(), Bounds(context)), Movement(1.1313708498984762, 0.84852813742385702, 0.10471999999999999));
    EXPECT_EQ(get_next_movement(Point(10, 0), MovementState(0, Point(), 0), OptPoint(), Bounds(context)), Movement(4, 0, 0));
    EXPECT_EQ(get_next_movement(Point(0, 10), MovementState(0, Point(), 0), OptPoint(), Bounds(context)), Movement(2.4492935982947064e-16, 3, 0.10471999999999999));
    EXPECT_EQ(get_next_movement(Point(10, 0), MovementState(0, Point(), M_PI / 2), OptPoint(), Bounds(context)), Movement(2.4492935982947064e-16, -3, -0.10471999999999999));
    EXPECT_EQ(get_next_movement(Point(-10, 0), MovementState(0, Point(), 0), OptPoint(), Bounds(context)), Movement(-3, 3.6739403974420594e-16, 0.10471999999999999));
    EXPECT_EQ(get_next_movement(Point(0, -10), MovementState(0, Point(), M_PI / 2), OptPoint(), Bounds(context)), Movement(-3, -3.6739403974420594e-16, -0.10471999999999999));
    EXPECT_EQ(get_next_movement(Point(10, 10), MovementState(0, Point(), 0), OptPoint(), Bounds(context)), Movement(2.8284271247461903, 2.1213203435596424, 0.10471999999999999));
}

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
    const Profiler profiler;
    const FullCache cache;
    const Context context(WIZARD, world, GAME, move, cache, profiler, Duration::max());
    const Path path({get_position(WIZARD), Point(1200, 1200)});
    const OptPoint look_target;
    const auto result = get_optimal_movement(context, path, look_target);
    ASSERT_FALSE(result.first.empty());
    EXPECT_EQ(result.first.size(), 72);
    EXPECT_EQ(result.second.size(), 71);
    EXPECT_EQ(result.first.back(), MovementState(71, Point(1200, 1200), 0.78539816339744828));
}

TEST(get_optimal_movement, only_for_me_not_direct) {
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
    const Path path({get_position(WIZARD), Point(1100, 1200), Point(1200, 1100)});
    const OptPoint look_target;
    const auto result = get_optimal_movement(context, path, look_target);
    ASSERT_FALSE(result.first.empty());
    EXPECT_EQ(result.first.size(), 94);
    EXPECT_EQ(result.second.size(), 93);
    EXPECT_EQ(result.first.back(), MovementState(93, Point(1200, 1100), -0.80858386837138518));
}

TEST(get_optimal_movement, only_for_me_with_hastened) {
    const model::Wizard wizard(
        1, // Id
        1000, // X
        1000, // Y
        0, // SpeedX
        0, // SpeedY
        M_PI / 4, // Angle
        model::FACTION_ACADEMY, // Faction
        35, // Radius
        100, // Life
        100, // MaxLife
        {model::Status(0, model::STATUS_HASTENED, 0, 0, 100)}, // Statuses
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
        {} // Trees
    );
    model::Move move;
    const Profiler profiler;
    const FullCache cache;
    const Context context(wizard, world, GAME, move, cache, profiler, Duration::max());
    const Path path({get_position(wizard), Point(1200, 1200)});
    const OptPoint look_target;
    const auto result = get_optimal_movement(context, path, look_target);
    ASSERT_FALSE(result.first.empty());
    EXPECT_EQ(result.first.size(), 56);
    EXPECT_EQ(result.second.size(), 55);
    EXPECT_EQ(result.first.back(), MovementState(55, Point(1200, 1200), 0.78539816339744828));
}

TEST(get_optimal_movement, with_static_barriers) {
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
    const Context context(wizard, world, GAME, move, cache, profiler, Duration::max());
    const Point target(1200, 1200);
    const auto step_size = 3;
    const auto path = get_optimal_path(context, target, step_size);
    EXPECT_EQ(path.size(), 29);
    const OptPoint look_target;
    const auto result = get_optimal_movement(context, path, look_target);
    ASSERT_FALSE(result.first.empty());
    EXPECT_EQ(result.first.size(), 80);
    EXPECT_EQ(result.second.size(), 79);
    EXPECT_EQ(result.first.back(), MovementState(79, Point(1200, 1200), 0.98120377803346293));
}

}
}
