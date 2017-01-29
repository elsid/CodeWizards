#include "common.hpp"

#include <simulation/simulator.hpp>
#include <simulation/minion_strategy.hpp>
#include <optimal_position.hpp>

#include <MyStrategy.h>

#include <gtest/gtest.h>

namespace strategy {
namespace simulation {
namespace tests {

using namespace testing;
using namespace strategy::tests;

TEST(simulation, move_wizard_for_one_tick) {
    const model::Wizard wizard(
        1, // Id
        2000, // X
        2000, // Y
        2, // SpeedX
        2, // SpeedY
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

    model::World world(
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

    Simulator simulator(GAME, world);

    simulator.next_tick();

    const auto& updated_wizard = *std::find_if(world.getWizards().begin(), world.getWizards().end(),
        [&] (const auto& v) { return v.getId() == wizard.getId(); });

    EXPECT_EQ(get_position(updated_wizard), Point(2002, 2002));
}

TEST(simulation, wizard_try_evade_projectile_and_fail) {
    model::Wizard self(
        1, // Id
        1000, // X
        2000, // Y
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

    const model::Projectile projectile(
        1, // Id
        1500, // X
        2000, // Y
        -40, // SpeedX
        0, // SpeedY
        0, // Angle
        model::FACTION_RENEGADES, // Faction
        10, // Radius
        model::PROJECTILE_MAGIC_MISSILE, // Type
        -1, // OwnerUnitId
        -1 // OwnerPlayerId
    );

    model::World world(
        0, // TickIndex
        20000, // TickCount
        4000, // Width
        4000, // Height
        {}, // Players
        {self}, // Wizards
        {}, // Minions
        {projectile}, // Projectiles
        {}, // Bonuses
        {}, // Buildings
        {} // Trees
    );

    Simulator simulator(GAME, world);
    MyStrategy my_strategy;

    for (int i = 0; i < 13; ++i) {
        model::Move move;

        self = *std::find_if(world.getWizards().begin(), world.getWizards().end(),
            [&] (const auto& v) { return v.getId() == self.getId(); });

        my_strategy.move(self, world, GAME, move);

        simulator.handle_wizard_move(self.getId(), move);
        simulator.next_tick();
    }

    const auto updated_projectile = std::find_if(world.getProjectiles().begin(), world.getProjectiles().end(),
        [&] (const auto& v) { return v.getId() == projectile.getId(); });

    EXPECT_EQ(get_position(self), Point(1007.1735815460353, 2039.5746567240087));
    EXPECT_EQ(self.getLife(), 99);
    EXPECT_EQ(updated_projectile, world.getProjectiles().end());
}

TEST(simulation, wizard_try_evade_projectile_and_succeed) {
    model::Wizard self(
        1, // Id
        1000, // X
        2006, // Y
        0, // SpeedX
        0, // SpeedY
        M_PI_2, // Angle
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

    const model::Projectile projectile(
        1, // Id
        1500, // X
        2000, // Y
        -40, // SpeedX
        0, // SpeedY
        0, // Angle
        model::FACTION_RENEGADES, // Faction
        10, // Radius
        model::PROJECTILE_MAGIC_MISSILE, // Type
        -1, // OwnerUnitId
        -1 // OwnerPlayerId
    );

    model::World world(
        0, // TickIndex
        20000, // TickCount
        4000, // Width
        4000, // Height
        {}, // Players
        {self}, // Wizards
        {}, // Minions
        {projectile}, // Projectiles
        {}, // Bonuses
        {}, // Buildings
        {} // Trees
    );

    Simulator simulator(GAME, world);
    MyStrategy my_strategy;

    for (int i = 0; i < 13; ++i) {
        model::Move move;

        self = *std::find_if(world.getWizards().begin(), world.getWizards().end(),
            [&] (const auto& v) { return v.getId() == self.getId(); });

        my_strategy.move(self, world, GAME, move);

        simulator.handle_wizard_move(self.getId(), move);
        simulator.next_tick();
    }

    const auto updated_projectile = std::find_if(world.getProjectiles().begin(), world.getProjectiles().end(),
        [&] (const auto& v) { return v.getId() == projectile.getId(); });

    EXPECT_EQ(get_position(self), Point(1008.315504368285, 2051.1340546153483));
    EXPECT_EQ(self.getLife(), 100);
    ASSERT_NE(updated_projectile, world.getProjectiles().end());
    EXPECT_EQ(get_position(*updated_projectile), Point(980, 2000));
    EXPECT_EQ(get_position(*updated_projectile).distance(Point(1500, 2000)), 520);
}

TEST(simulation, wizard_apply_cast_action_to_other_and_hit) {
    model::Wizard active_wizard(
        1, // Id
        1000, // X
        2000, // Y
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

    model::Wizard passive_wizard(
        2, // Id
        1505, // X
        2000, // Y
        0, // SpeedX
        0, // SpeedY
        M_PI, // Angle
        model::FACTION_RENEGADES, // Faction
        35, // Radius
        100, // Life
        100, // MaxLife
        {}, // Statuses
        2, // OwnerPlayerId
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

    model::World world(
        0, // TickIndex
        20000, // TickCount
        4000, // Width
        4000, // Height
        {}, // Players
        {active_wizard, passive_wizard}, // Wizards
        {}, // Minions
        {}, // Projectiles
        {}, // Bonuses
        {}, // Buildings
        {} // Trees
    );

    Simulator simulator(GAME, world);
    MyStrategy active_wizard_strategy;
    MyStrategy passive_wizard_strategy;

    for (int i = 0; i < 14; ++i) {
        model::Move active_wizard_move;

        active_wizard = *std::find_if(world.getWizards().begin(), world.getWizards().end(),
                [&] (const auto& v) { return v.getId() == active_wizard.getId(); });

        active_wizard_strategy.move(active_wizard, world, GAME, active_wizard_move);
        simulator.handle_wizard_move(active_wizard.getId(), active_wizard_move);

        model::Move passive_wizard_move;

        passive_wizard = *std::find_if(world.getWizards().begin(), world.getWizards().end(),
                [&] (const auto& v) { return v.getId() == passive_wizard.getId(); });

        passive_wizard_strategy.move(passive_wizard, world, GAME, passive_wizard_move);
        simulator.handle_wizard_move(passive_wizard.getId(), passive_wizard_move);

        simulator.next_tick();
    }

    EXPECT_EQ(get_position(active_wizard), Point(1045.2462041546494, 1999.5521338364892));
    EXPECT_EQ(get_position(passive_wizard), Point(1514.1268177486875, 2038.6877523657579));
    EXPECT_EQ(passive_wizard.getLife(), 88);
}

TEST(simulation, wizard_and_minion) {
    model::Wizard wizard(
        1, // Id
        1000, // X
        2000, // Y
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

    model::Minion minion(
        1, // Id
        1000 + 300, // X
        2000, // Y
        0, // SpeedX
        0, // SpeedY
        M_PI, // Angle
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

    model::World world(
        0, // TickIndex
        20000, // TickCount
        4000, // Width
        4000, // Height
        {}, // Players
        {wizard}, // Wizards
        {minion}, // Minions
        {}, // Projectiles
        {}, // Bonuses
        {}, // Buildings
        {} // Trees
    );

    Simulator simulator(GAME, world);
    MyStrategy wizard_strategy;
    MinionStrategy minion_strategy;

    for (int i = 0; i < 481; ++i) {
        model::Move wizard_move;

        wizard = *std::find_if(world.getWizards().begin(), world.getWizards().end(),
                [&] (const auto& v) { return v.getId() == wizard.getId(); });

        MinionMove minion_move;

        const auto minion_it = std::find_if(world.getMinions().begin(), world.getMinions().end(),
                [&] (const auto& v) { return v.getId() == minion.getId(); });

        if (minion_it == world.getMinions().end()) {
            break;
        }

        minion = *minion_it;

        wizard_strategy.move(wizard, world, GAME, wizard_move);
        minion_strategy.move(minion, world, GAME, minion_move);

        simulator.handle_wizard_move(wizard.getId(), wizard_move);
        simulator.handle_minion_move(minion.getId(), minion_move);

        simulator.next_tick();
    }

    EXPECT_EQ(get_position(wizard), Point(41.146940732030792, 1676.2096030634159));

    const auto minion_it = std::find_if(world.getMinions().begin(), world.getMinions().end(),
            [&] (const auto& v) { return v.getId() == minion.getId(); });

    EXPECT_EQ(minion_it, world.getMinions().end());
}

} // namespace tests
} // namespace simulation
} // namespace strategy
