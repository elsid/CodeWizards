#include "common.hpp"

#include <debug/output.hpp>
#include <MyStrategy.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace strategy {
namespace tests {

using namespace testing;

TEST(MyStrategy, initial) {
    static const model::Wizard self(
        1, // Id
        400, // X
        3600, // Y
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
    MyStrategy().move(self, world, GAME, move);
    EXPECT_DOUBLE_EQ(move.getCastAngle(), 0);
    EXPECT_DOUBLE_EQ(move.getMinCastDistance(), 0);
    EXPECT_DOUBLE_EQ(move.getSpeed(), 3.1040737685787283);
    EXPECT_DOUBLE_EQ(move.getStrafeSpeed(), -1.8921306501037571);
    EXPECT_DOUBLE_EQ(move.getTurn(), -0.10471999999999999);
    EXPECT_EQ(move.getAction(), model::_ACTION_UNKNOWN_);
    EXPECT_EQ(move.getSkillToLearn(), model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_1);
    EXPECT_EQ(move.getStatusTargetId(), -1);
}

TEST(MyStrategy, with_useless_message) {
    static const model::Wizard self(
        1, // Id
        400, // X
        3600, // Y
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
        {model::Message(model::_LANE_UNKNOWN_, model::_SKILL_UNKNOWN_, {})} // Messages
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
    MyStrategy().move(self, world, GAME, move);
    EXPECT_DOUBLE_EQ(move.getCastAngle(), 0);
    EXPECT_DOUBLE_EQ(move.getMinCastDistance(), 0);
    EXPECT_DOUBLE_EQ(move.getSpeed(), 3.1040737685787283);
    EXPECT_DOUBLE_EQ(move.getStrafeSpeed(), -1.8921306501037571);
    EXPECT_DOUBLE_EQ(move.getTurn(), -0.10471999999999999);
    EXPECT_EQ(move.getAction(), model::_ACTION_UNKNOWN_);
    EXPECT_EQ(move.getSkillToLearn(), model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_1);
    EXPECT_EQ(move.getStatusTargetId(), -1);
}

TEST(MyStrategy, with_useful_message) {
    static const model::Wizard self(
        1, // Id
        400, // X
        3600, // Y
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
        {model::Message(model::LANE_TOP, model::SKILL_HASTE, {})} // Messages
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
    MyStrategy().move(self, world, GAME, move);
    EXPECT_DOUBLE_EQ(move.getCastAngle(), 0);
    EXPECT_DOUBLE_EQ(move.getMinCastDistance(), 0);
    EXPECT_DOUBLE_EQ(move.getSpeed(), 3.1040737685787283);
    EXPECT_DOUBLE_EQ(move.getStrafeSpeed(), -1.8921306501037571);
    EXPECT_DOUBLE_EQ(move.getTurn(), -0.10471999999999999);
    EXPECT_EQ(move.getAction(), model::_ACTION_UNKNOWN_);
    EXPECT_EQ(move.getSkillToLearn(), model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_1);
    EXPECT_EQ(move.getStatusTargetId(), -1);
}

TEST(MyStrategy, with_near_enemy) {
    static const model::Wizard self(
        1, // Id
        2000, // X
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
    static const model::Wizard enemy(
        2, // Id
        2400, // X
        2010, // Y
        0, // SpeedX
        0, // SpeedY
        0, // Angle
        model::FACTION_RENEGADES, // Faction
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
    const model::World world(
        0, // TickIndex
        20000, // TickCount
        4000, // Width
        4000, // Height
        {}, // Players
        {self, enemy}, // Wizards
        {}, // Minions
        {}, // Projectiles
        {}, // Bonuses
        {}, // Buildings
        {} // Trees
    );
    model::Move move;
    MyStrategy().move(self, world, GAME, move);
    EXPECT_DOUBLE_EQ(move.getCastAngle(), 0.024994793618920159);
    EXPECT_DOUBLE_EQ(move.getMinCastDistance(), 355.12498047485116);
    EXPECT_DOUBLE_EQ(move.getSpeed(), -1.7292684299776877);
    EXPECT_DOUBLE_EQ(move.getStrafeSpeed(), 2.4514548123680564);
    EXPECT_DOUBLE_EQ(move.getTurn(), 0.024994793618920159);
    EXPECT_EQ(move.getAction(), model::ACTION_MAGIC_MISSILE);
    EXPECT_EQ(move.getSkillToLearn(), model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_1);
    EXPECT_EQ(move.getStatusTargetId(), -1);
}

}
}
