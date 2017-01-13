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
    EXPECT_EQ(get_shift(MovementState(0, Point(), M_PI / 2), Movement(1, 0, 0)), Point(0, 1));
    EXPECT_EQ(get_shift(MovementState(0, Point(), M_PI / 2), Movement(0, 1, 0)), Point(-1, 0));
    EXPECT_EQ(get_shift(MovementState(0, Point(), M_PI / 2), Movement(-1, 0, 0)), Point(0, -1));
    EXPECT_EQ(get_shift(MovementState(0, Point(), M_PI / 2), Movement(0, -1, 0)), Point(1, 0));
}

TEST(get_next_movement, all) {
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
    const auto bounds = make_unit_bounds(context.self(), context.game());
    EXPECT_EQ(get_next_movement(Point(), MovementState(0, Point(), 0), OptPoint(), bounds), Movement(0, 0, 0));
    EXPECT_EQ(get_next_movement(Point(1, 0), MovementState(0, Point(), 0), OptPoint(), bounds), Movement(1, 0, 0));
    EXPECT_EQ(get_next_movement(Point(0, 1), MovementState(0, Point(), 0), OptPoint(), bounds), Movement(0, 1, 0.10471999999999999));
    EXPECT_EQ(get_next_movement(Point(1, 0), MovementState(0, Point(), M_PI / 2), OptPoint(), bounds), Movement(0, -1, -0.10471999999999999));
    EXPECT_EQ(get_next_movement(Point(-1, 0), MovementState(0, Point(), 0), OptPoint(), bounds), Movement(-1, 0, 0.10471999999999999));
    EXPECT_EQ(get_next_movement(Point(0, -1), MovementState(0, Point(), 0), OptPoint(), bounds), Movement(0, -1, -0.10471999999999999));
    EXPECT_EQ(get_next_movement(Point(0, -1), MovementState(0, Point(), M_PI / 2), OptPoint(), bounds), Movement(-1, -0, -0.10471999999999999));
    EXPECT_EQ(get_next_movement(Point(1, 1), MovementState(0, Point(), 0), OptPoint(), bounds), Movement(1.1313708498984762, 0.84852813742385702, 0.10471999999999999));
    EXPECT_EQ(get_next_movement(Point(10, 0), MovementState(0, Point(), 0), OptPoint(), bounds), Movement(4, 0, 0));
    EXPECT_EQ(get_next_movement(Point(0, 10), MovementState(0, Point(), 0), OptPoint(), bounds), Movement(0, 3, 0.10471999999999999));
    EXPECT_EQ(get_next_movement(Point(10, 0), MovementState(0, Point(), M_PI / 2), OptPoint(), bounds), Movement(0, -3, -0.10471999999999999));
    EXPECT_EQ(get_next_movement(Point(-10, 0), MovementState(0, Point(), 0), OptPoint(), bounds), Movement(-3, 0, 0.10471999999999999));
    EXPECT_EQ(get_next_movement(Point(0, -10), MovementState(0, Point(), M_PI / 2), OptPoint(), bounds), Movement(-3, -0, -0.10471999999999999));
    EXPECT_EQ(get_next_movement(Point(10, 10), MovementState(0, Point(), 0), OptPoint(), bounds), Movement(2.8284271247461903, 2.1213203435596424, 0.10471999999999999));
}

TEST(get_optimal_movement, only_for_me) {
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
    const Path path({get_position(SELF), target});
    const OptPoint look_target;
    MovementsStates states;
    Movements movements;
    std::tie(states, movements) = get_optimal_movement(context, path, look_target);
    ASSERT_FALSE(states.empty());
    EXPECT_EQ(states.front(), MovementState(0, get_position(SELF), SELF.getAngle()));
    EXPECT_EQ(states.back(), MovementState(71, target, 0.78539816339744828));
    ASSERT_EQ(movements.size(), 71u);
    EXPECT_EQ(movements.front(), Movement(4, 0, 0));
    EXPECT_EQ(movements.back(), Movement(2.8427124746167487, 0, 0));
}

TEST(get_optimal_movement, only_for_me_to_my_position) {
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
    const Path path({get_position(SELF)});
    const OptPoint look_target;
    MovementsStates states;
    Movements movements;
    std::tie(states, movements) = get_optimal_movement(context, path, look_target);
    EXPECT_EQ(states, MovementsStates({MovementState(0, get_position(SELF), SELF.getAngle())}));
    EXPECT_EQ(movements, Movements());
}

TEST(get_optimal_movement, only_for_me_not_direct) {
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
    const Point final_target(1200, 1100);
    const Path path({get_position(SELF), Point(1100, 1200), final_target});
    const OptPoint look_target;
    MovementsStates states;
    Movements movements;
    std::tie(states, movements) = get_optimal_movement(context, path, look_target);
    ASSERT_EQ(states.size(), 94u);
    EXPECT_EQ(states.front(), MovementState(0, get_position(SELF), SELF.getAngle()));
    EXPECT_EQ(states.back(), MovementState(93, final_target, -0.80858386837138296));
    ASSERT_EQ(movements.size(), 93u);
    EXPECT_EQ(movements.front(), Movement(3.7947331922020551, 0.94868329805051366, 0.10471999999999999));
    EXPECT_EQ(movements.back(), Movement(3.329085623670311, 4.435233012416854e-15, 1.7763568394002505e-15));
}

TEST(get_optimal_movement, only_for_me_with_hastened) {
    const model::Wizard self(
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
    const Context context(self, world, GAME, move, cache, cache, profiler, Duration::max());
    const Point target(1200, 1200);
    const Path path({get_position(self), target});
    const OptPoint look_target;
    MovementsStates states;
    Movements movements;
    std::tie(states, movements) = get_optimal_movement(context, path, look_target);
    ASSERT_EQ(states.size(), 56u);
    EXPECT_EQ(states.front(), MovementState(0, get_position(self), self.getAngle()));
    EXPECT_EQ(states.back(), MovementState(55, target, 0.78539816339744828));
    ASSERT_EQ(movements.size(), 55u);
    EXPECT_EQ(movements.front(), Movement(5.2000000000000002, 0, 0));
    EXPECT_EQ(movements.back(), Movement(2.042712474626196, 0, 0));
}

TEST(get_optimal_movement, with_static_barriers) {
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
    const Context context(self, world, GAME, move, cache, cache, profiler, Duration::max());
    const Point target(1200, 1200);
    const auto path = GetOptimalPath().step_size(3)(context, target);
    const OptPoint look_target;
    MovementsStates states;
    Movements movements;
    std::tie(states, movements) = get_optimal_movement(context, path, look_target);
    ASSERT_EQ(states.size(), 80u);
    EXPECT_EQ(states.front(), MovementState(0, get_position(self), self.getAngle()));
    EXPECT_EQ(states.back(), MovementState(79, target, 0.97911486513429558));
    ASSERT_EQ(movements.size(), 79u);
    EXPECT_EQ(movements.front(), Movement(3.5777087639996634, 1.3416407864998738, 0.10471999999999999));
    EXPECT_EQ(movements.back(), Movement(0.76249861542367225, -6.1649531777595502e-14, -1.078026556911027e-13));
}

}
}
