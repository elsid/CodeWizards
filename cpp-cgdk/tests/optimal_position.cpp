#include "common.hpp"

#include <debug/output.hpp>
#include <optimal_position.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace strategy {
namespace tests {

using namespace testing;

TEST(GetPositionPenalty, get_elimination_score_for_me_and_enemy_wizard) {
    FullCache cache;
    model::World world;
    for (std::size_t i = 0; i < 6; ++i) {
        const model::Wizard enemy(
            2, // Id
            1100, // X
            1100, // Y
            0, // SpeedX
            0, // SpeedY
            0, // Angle
            model::FACTION_RENEGADES, // Faction
            35, // Radius
            100 - i * 4, // Life
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
        world = model::World(
            i * i, // TickIndex
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
        update_cache(cache, world);
    }
    model::Move move;
    const Profiler profiler;
    const Context context(SELF, world, GAME, move, cache, cache, profiler, Duration::max());
    const auto& target = world.getWizards()[0];
    const GetPositionPenalty<model::Wizard> get_position_penalty(context, &target, 1000);
    EXPECT_DOUBLE_EQ(get_position_penalty.get_elimination_score(Point(1000, 1000)), 0.32249090143872688);
}

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
    FullCache cache;
    update_cache(cache, world);
    const Context context(SELF, world, GAME, move, cache, cache, profiler, Duration::max());
    const auto& target = world.getWizards()[0];
    const auto max_distance = 1000;
    const auto result = get_optimal_position(context, &target, max_distance, OPTIMAL_POSITION_INITIAL_POINTS_COUNT,
                                             OPTIMAL_POSITION_MINIMIZE_MAX_FUNCTION_CALLS);
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 530.90221113197788);
    EXPECT_EQ(result, Point(596.61397629173155, 931.30006247777749));
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
    FullCache cache;
    update_cache(cache, world);
    const Context context(SELF, world, GAME, move, cache, cache, profiler, Duration::max());
    const auto& target = world.getWizards()[0];
    const auto max_distance = 1000;
    const auto result = get_optimal_position(context, &target, max_distance, OPTIMAL_POSITION_INITIAL_POINTS_COUNT,
                                             OPTIMAL_POSITION_MINIMIZE_MAX_FUNCTION_CALLS);
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 526.61940235890881);
    EXPECT_EQ(result, Point(598.72131956232136, 938.60706496406374));
}

TEST(get_optimal_position, for_me_with_low_life_and_enemy_wizard_with_max_cooldown) {
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
        GAME.getWizardActionCooldownTicks(), // RemainingActionCooldownTicks
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
    FullCache cache;
    update_cache(cache, world);
    const Context context(self, world, GAME, move, cache, cache, profiler, Duration::max());
    const auto& target = world.getWizards()[0];
    const auto max_distance = 1000;
    const auto result = get_optimal_position(context, &target, max_distance, OPTIMAL_POSITION_INITIAL_POINTS_COUNT,
                                             OPTIMAL_POSITION_MINIMIZE_MAX_FUNCTION_CALLS);
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 541.28857820362009);
    EXPECT_EQ(result, Point(752.67947148956921, 684.83524310349799));
}

TEST(get_optimal_position, for_me_with_low_life_and_enemy_wizard_with_half_cooldown) {
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
        GAME.getWizardActionCooldownTicks() * 0.5, // RemainingActionCooldownTicks
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
    FullCache cache;
    update_cache(cache, world);
    const Context context(self, world, GAME, move, cache, cache, profiler, Duration::max());
    const auto& target = world.getWizards()[0];
    const auto max_distance = 1000;
    const auto result = get_optimal_position(context, &target, max_distance, OPTIMAL_POSITION_INITIAL_POINTS_COUNT,
                                             OPTIMAL_POSITION_MINIMIZE_MAX_FUNCTION_CALLS);
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 565.54325682080207);
    EXPECT_EQ(result, Point(535.93868406171009, 1059.0853669520991));
}

TEST(get_optimal_position, for_me_with_half_life_and_enemy_wizard) {
    const model::Wizard self(
        1, // Id
        1000, // X
        1000, // Y
        0, // SpeedX
        0, // SpeedY
        M_PI / 4, // Angle
        model::FACTION_ACADEMY, // Faction
        35, // Radius
        50, // Life
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
        {enemy, self}, // Wizards
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
    const auto& target = world.getWizards()[0];
    const auto max_distance = 1000;
    const auto result = get_optimal_position(context, &target, max_distance, OPTIMAL_POSITION_INITIAL_POINTS_COUNT,
                                             OPTIMAL_POSITION_MINIMIZE_MAX_FUNCTION_CALLS);
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 546.0984091417846);
    EXPECT_EQ(result, Point(572.94567624576587, 1242.958778251477));
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
        {enemy, self}, // Wizards
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
    const auto& target = world.getWizards()[0];
    const auto max_distance = 1000;
    const auto result = get_optimal_position(context, &target, max_distance, OPTIMAL_POSITION_INITIAL_POINTS_COUNT,
                                             OPTIMAL_POSITION_MINIMIZE_MAX_FUNCTION_CALLS);
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 578.28554862595149);
    EXPECT_EQ(result, Point(558.50355800381237, 1302.9674334840126));
}

TEST(get_optimal_position, for_me_with_critical_life_and_enemy_wizard) {
    const model::Wizard self(
        1, // Id
        1000, // X
        1000, // Y
        0, // SpeedX
        0, // SpeedY
        M_PI / 4, // Angle
        model::FACTION_ACADEMY, // Faction
        35, // Radius
        1, // Life
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
        {enemy, self}, // Wizards
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
    const auto& target = world.getWizards()[0];
    const auto max_distance = 1000;
    const auto result = get_optimal_position(context, &target, max_distance, OPTIMAL_POSITION_INITIAL_POINTS_COUNT,
                                             OPTIMAL_POSITION_MINIMIZE_MAX_FUNCTION_CALLS);
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 609.80184515079839);
    EXPECT_EQ(result, Point(1589.9286372825877, 736.91843792372924));
}

TEST(get_optimal_position, for_me_with_low_life_and_minion_in_staff_range) {
    const model::Wizard self(
        1, // Id
        1000, // X
        1000, // Y
        0, // SpeedX
        0, // SpeedY
        M_PI / 4, // Angle
        model::FACTION_ACADEMY, // Faction
        35, // Radius
        1, // Life
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
    const model::Minion enemy(
        2, // Id
        1060, // X
        1000, // Y
        0, // SpeedX
        0, // SpeedY
        0, // Angle
        model::FACTION_RENEGADES, // Faction
        25, // Radius
        100, // Life
        100, // MaxLife
        {}, // Statuses
        model::MINION_ORC_WOODCUTTER, // Type
        400, // VisionRange
        12, // Damage
        60, // CooldownTicks
        0 // RemainingActionCooldownTicks
    );
    const model::World world(
        0, // TickIndex
        20000, // TickCount
        4000, // Width
        4000, // Height
        {}, // Players
        {self}, // Wizards
        {enemy}, // Minions
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
    const auto& target = world.getMinions()[0];
    const auto max_distance = 1000;
    const auto result = get_optimal_position(context, &target, max_distance, OPTIMAL_POSITION_INITIAL_POINTS_COUNT,
                                             OPTIMAL_POSITION_MINIMIZE_MAX_FUNCTION_CALLS);
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 545.09684471697494);
    EXPECT_EQ(result, Point(544.13287962852803, 1176.1013465026519));
}

TEST(get_optimal_position, for_me_with_full_life_and_enemy_building_with_inactive_cooldown) {
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
    const model::Building enemy(
        2, // Id
        1500, // X
        1000, // Y
        0, // SpeedX
        0, // SpeedY
        0, // Angle
        model::FACTION_RENEGADES, // Faction
        50, // Radius
        500, // Life
        500, // MaxLife
        {}, // Statuses
        model::BUILDING_GUARDIAN_TOWER, // Type
        600, // VisionRange
        600, // AttackRange
        36, // Damage
        240, // CooldownTicks
        0 // RemainingActionCooldownTicks
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
        {enemy}, // Buildings
        {} // Trees
    );
    model::Move move;
    const Profiler profiler;
    FullCache cache;
    update_cache(cache, world);
    const Context context(self, world, GAME, move, cache, cache, profiler, Duration::max());
    const auto& target = world.getBuildings()[0];
    const auto max_distance = 1000;
    const auto result = get_optimal_position(context, &target, max_distance, OPTIMAL_POSITION_INITIAL_POINTS_COUNT,
                                             OPTIMAL_POSITION_MINIMIZE_MAX_FUNCTION_CALLS);
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 539.38635015844079);
    EXPECT_EQ(result, Point(981.68600319717189, 1149.2924494256893));
}

TEST(get_optimal_position, for_me_with_full_life_and_enemy_building_with_active_cooldown) {
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
    const model::Building enemy(
        2, // Id
        1500, // X
        1000, // Y
        0, // SpeedX
        0, // SpeedY
        0, // Angle
        model::FACTION_RENEGADES, // Faction
        50, // Radius
        500, // Life
        500, // MaxLife
        {}, // Statuses
        model::BUILDING_GUARDIAN_TOWER, // Type
        600, // VisionRange
        600, // AttackRange
        36, // Damage
        240, // CooldownTicks
        240 // RemainingActionCooldownTicks
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
        {enemy}, // Buildings
        {} // Trees
    );
    model::Move move;
    const Profiler profiler;
    FullCache cache;
    update_cache(cache, world);
    const Context context(self, world, GAME, move, cache, cache, profiler, Duration::max());
    const auto& target = world.getBuildings()[0];
    const auto max_distance = 1000;
    const auto result = get_optimal_position(context, &target, max_distance, OPTIMAL_POSITION_INITIAL_POINTS_COUNT,
                                             OPTIMAL_POSITION_MINIMIZE_MAX_FUNCTION_CALLS);
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 526.47919825695931);
    EXPECT_EQ(result, Point(976.9767861302056, 939.77488936828831));
}

TEST(get_optimal_position, for_me_with_low_life_and_enemy_building_with_inactive_cooldown) {
    const model::Wizard self(
        1, // Id
        1000, // X
        1000, // Y
        0, // SpeedX
        0, // SpeedY
        M_PI / 4, // Angle
        model::FACTION_ACADEMY, // Faction
        35, // Radius
        36, // Life
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
    const model::Building enemy(
        2, // Id
        1500, // X
        1000, // Y
        0, // SpeedX
        0, // SpeedY
        0, // Angle
        model::FACTION_RENEGADES, // Faction
        50, // Radius
        500, // Life
        500, // MaxLife
        {}, // Statuses
        model::BUILDING_GUARDIAN_TOWER, // Type
        600, // VisionRange
        600, // AttackRange
        36, // Damage
        240, // CooldownTicks
        0 // RemainingActionCooldownTicks
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
        {enemy}, // Buildings
        {} // Trees
    );
    model::Move move;
    const Profiler profiler;
    FullCache cache;
    update_cache(cache, world);
    const Context context(self, world, GAME, move, cache, cache, profiler, Duration::max());
    const auto& target = world.getBuildings()[0];
    const auto max_distance = 1000;
    const auto result = get_optimal_position(context, &target, max_distance, OPTIMAL_POSITION_INITIAL_POINTS_COUNT,
                                             OPTIMAL_POSITION_MINIMIZE_MAX_FUNCTION_CALLS);
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 627.9999493416301);
    EXPECT_EQ(result, Point(872.0000634999102, 1000.1269998943077));
}

TEST(get_optimal_position, for_me_low_life_and_enemy_building_with_half_cooldown) {
    const model::Wizard self(
        1, // Id
        1000, // X
        1000, // Y
        0, // SpeedX
        0, // SpeedY
        M_PI / 4, // Angle
        model::FACTION_ACADEMY, // Faction
        35, // Radius
        36, // Life
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
    const model::Building enemy(
        2, // Id
        1500, // X
        1000, // Y
        0, // SpeedX
        0, // SpeedY
        0, // Angle
        model::FACTION_RENEGADES, // Faction
        50, // Radius
        500, // Life
        500, // MaxLife
        {}, // Statuses
        model::BUILDING_GUARDIAN_TOWER, // Type
        600, // VisionRange
        600, // AttackRange
        36, // Damage
        240, // CooldownTicks
        120 // RemainingActionCooldownTicks
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
        {enemy}, // Buildings
        {} // Trees
    );
    model::Move move;
    const Profiler profiler;
    FullCache cache;
    update_cache(cache, world);
    const Context context(self, world, GAME, move, cache, cache, profiler, Duration::max());
    const auto& target = world.getBuildings()[0];
    const auto max_distance = 1000;
    const auto result = get_optimal_position(context, &target, max_distance, OPTIMAL_POSITION_INITIAL_POINTS_COUNT,
                                             OPTIMAL_POSITION_MINIMIZE_MAX_FUNCTION_CALLS);
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 567.59910006177893);
    EXPECT_EQ(result, Point(933.12369383474254, 971.3644993168524));
}

TEST(get_optimal_position, for_me_low_life_and_enemy_building_with_active_cooldown) {
    const model::Wizard self(
        1, // Id
        1000, // X
        1000, // Y
        0, // SpeedX
        0, // SpeedY
        M_PI / 4, // Angle
        model::FACTION_ACADEMY, // Faction
        35, // Radius
        36, // Life
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
    const model::Building enemy(
        2, // Id
        1500, // X
        1000, // Y
        0, // SpeedX
        0, // SpeedY
        0, // Angle
        model::FACTION_RENEGADES, // Faction
        50, // Radius
        500, // Life
        500, // MaxLife
        {}, // Statuses
        model::BUILDING_GUARDIAN_TOWER, // Type
        600, // VisionRange
        600, // AttackRange
        36, // Damage
        240, // CooldownTicks
        240 // RemainingActionCooldownTicks
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
        {enemy}, // Buildings
        {} // Trees
    );
    model::Move move;
    const Profiler profiler;
    FullCache cache;
    update_cache(cache, world);
    const Context context(self, world, GAME, move, cache, cache, profiler, Duration::max());
    const auto& target = world.getBuildings()[0];
    const auto max_distance = 1000;
    const auto result = get_optimal_position(context, &target, max_distance, OPTIMAL_POSITION_INITIAL_POINTS_COUNT,
                                             OPTIMAL_POSITION_MINIMIZE_MAX_FUNCTION_CALLS);
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 535.54639650655554);
    EXPECT_EQ(result, Point(964.61205051392142, 986.9736284311806));
}

}
}
