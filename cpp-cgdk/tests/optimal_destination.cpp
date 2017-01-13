#include "common.hpp"

#include <debug/output.hpp>
#include <optimal_destination.hpp>

#include <gtest/gtest.h>

namespace strategy {
namespace tests {

using namespace testing;

TEST(get_optimal_destination, for_all_default) {
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
    WorldGraph graph(GAME);
    EXPECT_EQ(get_optimal_destination(context, graph, model::_LANE_UNKNOWN_, SELF).id, 46u);
}

const model::Building FIRST_MIDDLE_ENEMY_TOWER(
    1, // Id
    4000 - 902, // X
    4000 - 2768, // Y
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

const model::Building FIRST_TOP_ENEMY_TOWER(
    2, // Id
    4000 - 2312, // X
    4000 - 3950, // Y
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

const model::Building FIRST_BOTTOM_ENEMY_TOWER(
    3, // Id
    4000 - 350, // X
    4000 - 1656.75, // Y
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

TEST(get_optimal_destination, for_me_with_low_life_at_middle_lane) {
    const model::Wizard self(
        1, // Id
        4000 - 902, // X
        4000 - 2768, // Y
        0, // SpeedX
        0, // SpeedY
        0, // Angle
        model::FACTION_ACADEMY, // Faction
        35, // Radius
        33, // Life
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
        {FIRST_MIDDLE_ENEMY_TOWER, FIRST_TOP_ENEMY_TOWER, FIRST_BOTTOM_ENEMY_TOWER}, // Buildings
        {} // Trees
    );
    model::Move move;
    const Profiler profiler;
    FullCache cache;
    update_cache(cache, world);
    const Context context(self, world, GAME, move, cache, cache, profiler, Duration::max());
    WorldGraph graph(GAME);
    EXPECT_EQ(get_optimal_destination(context, graph, model::_LANE_UNKNOWN_, self).id, 19u);
}

TEST(get_optimal_destination, for_me_with_low_life_with_middle_target_lane_at_middle_lane) {
    const model::Wizard self(
        1, // Id
        4000 - 902, // X
        4000 - 2768, // Y
        0, // SpeedX
        0, // SpeedY
        0, // Angle
        model::FACTION_ACADEMY, // Faction
        35, // Radius
        33, // Life
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
        {FIRST_MIDDLE_ENEMY_TOWER, FIRST_TOP_ENEMY_TOWER, FIRST_BOTTOM_ENEMY_TOWER}, // Buildings
        {} // Trees
    );
    model::Move move;
    const Profiler profiler;
    FullCache cache;
    update_cache(cache, world);
    const Context context(self, world, GAME, move, cache, cache, profiler, Duration::max());
    WorldGraph graph(GAME);
    EXPECT_EQ(get_optimal_destination(context, graph, model::LANE_MIDDLE, self).id, 19u);
}

TEST(get_optimal_destination, for_me_with_low_life_at_top_lane) {
    const model::Wizard self(
        1, // Id
        4000 - 2312, // X
        4000 - 3950, // Y
        0, // SpeedX
        0, // SpeedY
        0, // Angle
        model::FACTION_ACADEMY, // Faction
        35, // Radius
        33, // Life
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
        {FIRST_MIDDLE_ENEMY_TOWER, FIRST_TOP_ENEMY_TOWER, FIRST_BOTTOM_ENEMY_TOWER}, // Buildings
        {} // Trees
    );
    model::Move move;
    const Profiler profiler;
    FullCache cache;
    update_cache(cache, world);
    const Context context(self, world, GAME, move, cache, cache, profiler, Duration::max());
    WorldGraph graph(GAME);
    EXPECT_EQ(get_optimal_destination(context, graph, model::_LANE_UNKNOWN_, self).id, 0u);
}

TEST(get_optimal_destination, for_me_with_low_life_at_bottom_lane) {
    const model::Wizard self(
        1, // Id
        4000 - 350, // X
        4000 - 1656.75, // Y
        0, // SpeedX
        0, // SpeedY
        0, // Angle
        model::FACTION_ACADEMY, // Faction
        35, // Radius
        33, // Life
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
        {FIRST_MIDDLE_ENEMY_TOWER, FIRST_TOP_ENEMY_TOWER, FIRST_BOTTOM_ENEMY_TOWER}, // Buildings
        {} // Trees
    );
    model::Move move;
    const Profiler profiler;
    FullCache cache;
    update_cache(cache, world);
    const Context context(self, world, GAME, move, cache, cache, profiler, Duration::max());
    WorldGraph graph(GAME);
    EXPECT_EQ(get_optimal_destination(context, graph, model::_LANE_UNKNOWN_, self).id, 37u);
}

}
}
