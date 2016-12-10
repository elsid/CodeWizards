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
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 528.1949086649729);
    EXPECT_EQ(result, Point(588.68315373316841, 1232.4573299721096));
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
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 524.26129325238276);
    EXPECT_EQ(result, Point(1592.3036005237484, 919.75830528422546));
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
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 536.42069971951446);
    EXPECT_EQ(result, Point(585.35495397382465, 1251.2866275924259));
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
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 561.56955298404432);
    EXPECT_EQ(result, Point(577.09922283716048, 895.21928781939016));
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
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 541.38447417535406);
    EXPECT_EQ(result, Point(589.47850673380242, 919.8196631389086));
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
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 574.98298694460175);
    EXPECT_EQ(result, Point(1488.0997047136304, 675.75472368342287));
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
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 609.23890669484342);
    EXPECT_EQ(result, Point(608.09419929601256, 740.55497128977129));
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
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 149.86879706230724);
    EXPECT_EQ(result, Point(1209.860902263006, 1001.5382801502985));
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
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 572.58655531648606);
    EXPECT_EQ(result, Point(927.41619075629035, 1001.7733361673647));
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
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 562.86756168179897);
    EXPECT_EQ(result, Point(937.29751368209088, 986.36901011455484));
}

TEST(get_optimal_position, for_me_with_full_life_and_max_cooldown_and_enemy_building_with_active_cooldown) {
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
    const auto result = get_optimal_position(context, &target, max_distance, OPTIMAL_POSITION_INITIAL_POINTS_COUNT,
                                             OPTIMAL_POSITION_MINIMIZE_MAX_FUNCTION_CALLS);
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 755.99991550605432);
    EXPECT_EQ(result, Point(744.00012749983898, 1000.2549998069766));
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
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 653.8750057258261);
    EXPECT_EQ(result, Point(849.29420666380122, 1064.3000282399412));
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
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 629.11158920531977);
    EXPECT_EQ(result, Point(871.13916458924393, 982.23933112383077));
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
    EXPECT_DOUBLE_EQ(result.distance(get_position(target)), 591.9105790790361);
    EXPECT_EQ(result, Point(908.10256915136222, 996.05475216724267));
}

}
}
