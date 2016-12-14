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
        - 3 * M_PI / 4, // Angle
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
    const auto result = get_optimal_position(context, &target, max_distance);
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 186.41205771089261);
    EXPECT_EQ(result, Point(968.23170136185092, 968.14185374419708));
}

TEST(get_optimal_position, for_me_and_enemy_wizard_with_active_cooldown) {
    const model::Wizard enemy(
        2, // Id
        1100, // X
        1100, // Y
        0, // SpeedX
        0, // SpeedY
        - 3 * M_PI / 4, // Angle
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
    const auto result = get_optimal_position(context, &target, max_distance);
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 103.10336436506569);
    EXPECT_EQ(result, Point(1020.5252804048062, 1034.3183991618596));
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
        - 3 * M_PI / 4, // Angle
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
    const auto result = get_optimal_position(context, &target, max_distance);
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 106.2122147495307);
    EXPECT_EQ(result, Point(1027.3498616346585, 1022.5210224803066));
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
        - 3 * M_PI / 4, // Angle
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
    const auto result = get_optimal_position(context, &target, max_distance);
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 1113.7305595367095);
    EXPECT_EQ(result, Point(34.068783939415297, 777.2208156129725));
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
        - 3 * M_PI / 4, // Angle
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
    const auto result = get_optimal_position(context, &target, max_distance);
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 341.3699323991932);
    EXPECT_EQ(result, Point(835.57463947851829, 884.10034863549652));
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
        - 3 * M_PI / 4, // Angle
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
    const auto result = get_optimal_position(context, &target, max_distance);
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 589.06927035498688);
    EXPECT_EQ(result, Point(641.35863750480473, 730.33703744477259));
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
        - 3 * M_PI / 4, // Angle
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
    const auto result = get_optimal_position(context, &target, max_distance);
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 614.81884909632822);
    EXPECT_EQ(result, Point(565.03592215124286, 796.98902228539225));
}

TEST(get_optimal_position, for_me_with_low_life_and_minion_in_staff_range) {
    const model::Wizard self(
        1, // Id
        1000, // X
        1000, // Y
        0, // SpeedX
        0, // SpeedY
        0, // Angle
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
    const auto result = get_optimal_position(context, &target, max_distance);
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 150.72723938661073);
    EXPECT_EQ(result, Point(916.45255931515806, 954.03443717367918));
}

TEST(get_optimal_position, for_me_with_full_life_and_enemy_building_with_inactive_cooldown) {
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
    const auto result = get_optimal_position(context, &target, max_distance);
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 448.01389611945405);
    EXPECT_EQ(result, Point(1052.1778018265484, 1013.1045770336465));
}

TEST(get_optimal_position, for_me_with_full_life_and_enemy_building_with_active_cooldown) {
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
    const auto result = get_optimal_position(context, &target, max_distance);
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 104.32730853917285);
    EXPECT_EQ(result, Point(1407.5895843021426, 1048.420061726282));
}

TEST(get_optimal_position, for_me_with_full_life_and_max_cooldown_and_enemy_building_with_active_cooldown) {
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
        30, // RemainingActionCooldownTicks
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
    const auto result = get_optimal_position(context, &target, max_distance);
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 500);
    EXPECT_EQ(result, Point(1000, 1000));
}

TEST(get_optimal_position, for_me_with_low_life_and_enemy_building_with_inactive_cooldown) {
    const model::Wizard self(
        1, // Id
        1000, // X
        1000, // Y
        0, // SpeedX
        0, // SpeedY
        0, // Angle
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
    const auto result = get_optimal_position(context, &target, max_distance);
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 675.24864638220458);
    EXPECT_EQ(result, Point(824.75246057203378, 998.77732364185999));
}

TEST(get_optimal_position, for_me_low_life_and_enemy_building_with_half_cooldown) {
    const model::Wizard self(
        1, // Id
        1000, // X
        1000, // Y
        0, // SpeedX
        0, // SpeedY
        0, // Angle
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
    const auto result = get_optimal_position(context, &target, max_distance);
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 647.41965472230061);
    EXPECT_EQ(result, Point(860.66248995850026, 898.02079832300046));
}

TEST(get_optimal_position, for_me_low_life_and_enemy_building_with_active_cooldown) {
    const model::Wizard self(
        1, // Id
        1000, // X
        1000, // Y
        0, // SpeedX
        0, // SpeedY
        0, // Angle
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
    const auto result = get_optimal_position(context, &target, max_distance);
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 105.31413340329399);
    EXPECT_EQ(result, Point(1401.2630827824755, 963.36548249442171));
}

}
}
