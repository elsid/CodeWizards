#include "common.hpp"

#include <debug/output.hpp>
#include <optimal_position.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace strategy {
namespace tests {

using namespace testing;

TEST(get_optimal_position, for_me_and_enemy_wizard) {
    const model::Wizard enemy(
        2, // Id
        1100, // X
        1100, // Y
        0, // SpeedX
        0, // SpeedY
        0, // Angle
        model::FACTION_RENEGADES, // Faction
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
        true, // Master
        {} // Messages
    );
    const model::World world(
        0, // TickIndex
        20000, // TickCount
        4000, // Width
        4000, // Height
        {}, // Players
        {enemy, SELF}, // Wizards
        {}, // Minions
        {}, // Projectiles
        {}, // Bonuses
        {}, // Buildings
        {} // Trees
    );
    model::Move move;
    const Profiler profiler;
    const FullCache cache;
    const Context context(SELF, world, GAME, move, cache, cache, profiler, Duration::max());
    const auto& target = world.getWizards()[0];
    const auto result = get_optimal_position(context, &target, 1000, OPTIMAL_POSITION_INITIAL_POINTS_COUNT,
                                             OPTIMAL_POSITION_MINIMIZE_MAX_FUNCTION_CALLS);
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 557.21637798129905);
    EXPECT_EQ(result, Point(1545.9859150247987, 765.95411170006753));
}

TEST(get_optimal_position, for_me_and_enemy_wizard_with_active_cooldown) {
    const model::Wizard enemy(
        2, // Id
        1100, // X
        1100, // Y
        0, // SpeedX
        0, // SpeedY
        0, // Angle
        model::FACTION_RENEGADES, // Faction
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
        30, // RemainingActionCooldownTicks
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
        {enemy, SELF}, // Wizards
        {}, // Minions
        {}, // Projectiles
        {}, // Bonuses
        {}, // Buildings
        {} // Trees
    );
    model::Move move;
    const Profiler profiler;
    const FullCache cache;
    const Context context(SELF, world, GAME, move, cache, cache, profiler, Duration::max());
    const auto& target = world.getWizards()[0];
    const auto result = get_optimal_position(context, &target, 1000, OPTIMAL_POSITION_INITIAL_POINTS_COUNT,
                                             OPTIMAL_POSITION_MINIMIZE_MAX_FUNCTION_CALLS);
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 505.45528612542472);
    EXPECT_EQ(result, Point(1577.3307129158077, 933.74586688018326));
}

TEST(get_optimal_position, for_me_with_low_life_and_enemy_wizard) {
    const model::Wizard self(
        1, // Id
        1000, // X
        1000, // Y
        0, // SpeedX
        0, // SpeedY
        M_PI / 4, // Angle
        model::FACTION_ACADEMY, // Faction
        35, // Radius
        GAME.getMagicMissileDirectDamage(), // Life
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
    const model::Wizard enemy(
        2, // Id
        1100, // X
        1100, // Y
        0, // SpeedX
        0, // SpeedY
        0, // Angle
        model::FACTION_RENEGADES, // Faction
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
        30, // RemainingActionCooldownTicks
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
        {enemy, self}, // Wizards
        {}, // Minions
        {}, // Projectiles
        {}, // Bonuses
        {}, // Buildings
        {} // Trees
    );
    model::Move move;
    const Profiler profiler;
    const FullCache cache;
    const Context context(self, world, GAME, move, cache, cache, profiler, Duration::max());
    const auto& target = world.getWizards()[0];
    const auto result = get_optimal_position(context, &target, 1000, OPTIMAL_POSITION_INITIAL_POINTS_COUNT,
                                             OPTIMAL_POSITION_MINIMIZE_MAX_FUNCTION_CALLS);
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 1545.1872845280743);
    EXPECT_EQ(result, Point(-379.0592719905103, 652.80047606672724));
}

}
}
