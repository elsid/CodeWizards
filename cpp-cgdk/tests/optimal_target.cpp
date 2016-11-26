#include "common.hpp"

#include <debug/output.hpp>
#include <optimal_target.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace strategy {
namespace tests {

using namespace testing;

TEST(get_optimal_target, for_all_default) {
    const model::Wizard wizard;
    const model::World world;
    const model::Game game;
    model::Move move;
    const Context context {wizard, world, game, move};
    EXPECT_FALSE(get_optimal_target(context, 0).has_value());
}

TEST(get_optimal_target, for_me_and_enemy_wizard) {
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
        {enemy, WIZARD}, // Wizards
        {}, // Minions
        {}, // Projectiles
        {}, // Bonuses
        {}, // Buildings
        {} // Trees
    );
    model::Move move;
    const Context context {WIZARD, world, GAME, move};
    const auto result = get_optimal_target(context, 1000);
    EXPECT_EQ(result.wizard(), &world.getWizards()[0]);
}

}
}
