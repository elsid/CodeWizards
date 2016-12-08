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
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 535.31887170012953);
    EXPECT_EQ(result, Point(602.42763834680727, 902.54610837675523));
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
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 531.03023622804938);
    EXPECT_EQ(result, Point(694.24199332304465, 757.43095322845966));
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
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 549.13906799947745);
    EXPECT_EQ(result, Point(1495.564790314945, 719.1034094843742));
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
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 570.78718098479544);
    EXPECT_EQ(result, Point(1531.2017548525992, 726.01730977409045));
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
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 551.04758016882988);
    EXPECT_EQ(result, Point(697.19654432823916, 723.96434782216352));
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
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 582.00042150892477);
    EXPECT_EQ(result, Point(521.49084863494045, 1036.34740835249));
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
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 610.27568085968073);
    EXPECT_EQ(result, Point(584.3540983091824, 773.58659537603148));
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
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 553.33481938490661);
    EXPECT_EQ(result, Point(1567.3172987546372, 1220.9266410554087));
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
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 543.70972307810109);
    EXPECT_EQ(result, Point(980.63161610051725, 1160.8625027012351));
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
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 531.04633441661622);
    EXPECT_EQ(result, Point(968.95505854814348, 1001.2163287873595));
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
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 631.0369762544633);
    EXPECT_EQ(result, Point(871.29273569270799, 1054.1741747294498));
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
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 574.64103572399529);
    EXPECT_EQ(result, Point(925.39586259721466, 993.48807113467785));
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
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 542.11165297355251);
    EXPECT_EQ(result, Point(957.95300148282195, 991.62768322172383));
}

}
}
