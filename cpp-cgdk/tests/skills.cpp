#include "common.hpp"

#include <skills.hpp>

#include <gtest/gtest.h>

namespace strategy {
namespace tests {

using namespace testing;

TEST(get_next_skill_to_learn, from_begin_should_return_first_in_line) {
    EXPECT_EQ(get_next_skill_to_learn(SELF, model::SKILL_RANGE_BONUS_PASSIVE_1), model::SKILL_RANGE_BONUS_PASSIVE_1);
    EXPECT_EQ(get_next_skill_to_learn(SELF, model::SKILL_RANGE_BONUS_AURA_1), model::SKILL_RANGE_BONUS_PASSIVE_1);
    EXPECT_EQ(get_next_skill_to_learn(SELF, model::SKILL_RANGE_BONUS_PASSIVE_2), model::SKILL_RANGE_BONUS_PASSIVE_1);
    EXPECT_EQ(get_next_skill_to_learn(SELF, model::SKILL_RANGE_BONUS_AURA_2), model::SKILL_RANGE_BONUS_PASSIVE_1);
    EXPECT_EQ(get_next_skill_to_learn(SELF, model::SKILL_ADVANCED_MAGIC_MISSILE), model::SKILL_RANGE_BONUS_PASSIVE_1);

    EXPECT_EQ(get_next_skill_to_learn(SELF, model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_1), model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_1);
    EXPECT_EQ(get_next_skill_to_learn(SELF, model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_1), model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_1);
    EXPECT_EQ(get_next_skill_to_learn(SELF, model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_2), model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_1);
    EXPECT_EQ(get_next_skill_to_learn(SELF, model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_2), model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_1);
    EXPECT_EQ(get_next_skill_to_learn(SELF, model::SKILL_FROST_BOLT), model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_1);

    EXPECT_EQ(get_next_skill_to_learn(SELF, model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_1), model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_1);
    EXPECT_EQ(get_next_skill_to_learn(SELF, model::SKILL_STAFF_DAMAGE_BONUS_AURA_1), model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_1);
    EXPECT_EQ(get_next_skill_to_learn(SELF, model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_2), model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_1);
    EXPECT_EQ(get_next_skill_to_learn(SELF, model::SKILL_STAFF_DAMAGE_BONUS_AURA_2), model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_1);
    EXPECT_EQ(get_next_skill_to_learn(SELF, model::SKILL_FIREBALL), model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_1);

    EXPECT_EQ(get_next_skill_to_learn(SELF, model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_1), model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_1);
    EXPECT_EQ(get_next_skill_to_learn(SELF, model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_1), model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_1);
    EXPECT_EQ(get_next_skill_to_learn(SELF, model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_2), model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_1);
    EXPECT_EQ(get_next_skill_to_learn(SELF, model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_2), model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_1);
    EXPECT_EQ(get_next_skill_to_learn(SELF, model::SKILL_HASTE), model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_1);

    EXPECT_EQ(get_next_skill_to_learn(SELF, model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_1), model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_1);
    EXPECT_EQ(get_next_skill_to_learn(SELF, model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_1), model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_1);
    EXPECT_EQ(get_next_skill_to_learn(SELF, model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_2), model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_1);
    EXPECT_EQ(get_next_skill_to_learn(SELF, model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_2), model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_1);
    EXPECT_EQ(get_next_skill_to_learn(SELF, model::SKILL_SHIELD), model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_1);
}

const model::Wizard wizard_with_skill(model::SkillType skill) {
    return model::Wizard(
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
        {skill}, // Skills
        0, // RemainingActionCooldownTicks
        {0, 0, 0, 0, 0, 0, 0}, // RemainingCooldownTicksByAction
        true, // Master
        {} // Messages
    );
}

TEST(get_next_skill_to_learn, from_previous_should_return_target) {
    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_RANGE_BONUS_PASSIVE_1), model::SKILL_RANGE_BONUS_AURA_1), model::SKILL_RANGE_BONUS_AURA_1);
    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_RANGE_BONUS_AURA_1), model::SKILL_RANGE_BONUS_PASSIVE_2), model::SKILL_RANGE_BONUS_PASSIVE_2);
    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_RANGE_BONUS_PASSIVE_2), model::SKILL_RANGE_BONUS_AURA_2), model::SKILL_RANGE_BONUS_AURA_2);
    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_RANGE_BONUS_AURA_2), model::SKILL_ADVANCED_MAGIC_MISSILE), model::SKILL_ADVANCED_MAGIC_MISSILE);

    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_1), model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_1), model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_1);
    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_1), model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_2), model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_2);
    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_2), model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_2), model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_2);
    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_2), model::SKILL_FROST_BOLT), model::SKILL_FROST_BOLT);

    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_1), model::SKILL_STAFF_DAMAGE_BONUS_AURA_1), model::SKILL_STAFF_DAMAGE_BONUS_AURA_1);
    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_STAFF_DAMAGE_BONUS_AURA_1), model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_2), model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_2);
    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_2), model::SKILL_STAFF_DAMAGE_BONUS_AURA_2), model::SKILL_STAFF_DAMAGE_BONUS_AURA_2);
    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_STAFF_DAMAGE_BONUS_AURA_2), model::SKILL_FIREBALL), model::SKILL_FIREBALL);

    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_1), model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_1), model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_1);
    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_1), model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_1), model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_1);
    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_2), model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_2), model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_2);
    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_2), model::SKILL_HASTE), model::SKILL_HASTE);

    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_1), model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_1), model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_1);
    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_1), model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_2), model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_2);
    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_2), model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_2), model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_2);
    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_2), model::SKILL_SHIELD), model::SKILL_SHIELD);
}

TEST(get_next_skill_to_learn, from_same_level_should_return_unknown) {
    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_RANGE_BONUS_PASSIVE_1), model::SKILL_RANGE_BONUS_PASSIVE_1), model::_SKILL_UNKNOWN_);
    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_RANGE_BONUS_AURA_1), model::SKILL_RANGE_BONUS_AURA_1), model::_SKILL_UNKNOWN_);
    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_RANGE_BONUS_PASSIVE_2), model::SKILL_RANGE_BONUS_PASSIVE_2), model::_SKILL_UNKNOWN_);
    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_RANGE_BONUS_AURA_2), model::SKILL_RANGE_BONUS_AURA_2), model::_SKILL_UNKNOWN_);
    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_ADVANCED_MAGIC_MISSILE), model::SKILL_ADVANCED_MAGIC_MISSILE), model::_SKILL_UNKNOWN_);

    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_1), model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_1), model::_SKILL_UNKNOWN_);
    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_1), model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_1), model::_SKILL_UNKNOWN_);
    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_2), model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_2), model::_SKILL_UNKNOWN_);
    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_2), model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_2), model::_SKILL_UNKNOWN_);
    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_FROST_BOLT), model::SKILL_FROST_BOLT), model::_SKILL_UNKNOWN_);

    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_1), model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_1), model::_SKILL_UNKNOWN_);
    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_STAFF_DAMAGE_BONUS_AURA_1), model::SKILL_STAFF_DAMAGE_BONUS_AURA_1), model::_SKILL_UNKNOWN_);
    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_2), model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_2), model::_SKILL_UNKNOWN_);
    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_STAFF_DAMAGE_BONUS_AURA_2), model::SKILL_STAFF_DAMAGE_BONUS_AURA_2), model::_SKILL_UNKNOWN_);
    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_FIREBALL), model::SKILL_FIREBALL), model::_SKILL_UNKNOWN_);

    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_1), model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_1), model::_SKILL_UNKNOWN_);
    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_1), model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_1), model::_SKILL_UNKNOWN_);
    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_2), model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_2), model::_SKILL_UNKNOWN_);
    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_2), model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_2), model::_SKILL_UNKNOWN_);
    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_HASTE), model::SKILL_HASTE), model::_SKILL_UNKNOWN_);

    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_1), model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_1), model::_SKILL_UNKNOWN_);
    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_1), model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_1), model::_SKILL_UNKNOWN_);
    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_2), model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_2), model::_SKILL_UNKNOWN_);
    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_2), model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_2), model::_SKILL_UNKNOWN_);
    EXPECT_EQ(get_next_skill_to_learn(wizard_with_skill(model::SKILL_SHIELD), model::SKILL_SHIELD), model::_SKILL_UNKNOWN_);
}

TEST(get_opposite_skill, for_same_level_should_return_unknown) {
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
        {60, 60, 60, 60, 60, 60, 60}, // RemainingCooldownTicksByAction
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
    EXPECT_EQ(get_opposite_skill(context), std::make_pair(model::_SKILL_UNKNOWN_, 0));
}

TEST(get_opposite_skill, for_bigger_level_with_fireball_should_return_frost_bolt) {
    const model::Wizard self(
        1, // Id
        0, // X
        0, // Y
        0, // SpeedX
        0, // SpeedY
        0, // Angle
        model::FACTION_ACADEMY, // Faction
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
        {60, 60, 60, 60, 60, 60, 60}, // RemainingCooldownTicksByAction
        true, // Master
        {} // Messages
    );
    const model::Wizard enemy(
        2, // Id
        0, // X
        0, // Y
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
        1, // Level
        {model::SKILL_FIREBALL}, // Skills
        30, // RemainingActionCooldownTicks
        {60, 60, 60, 60, 60, 60, 60}, // RemainingCooldownTicksByAction
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
    EXPECT_EQ(get_opposite_skill(context), std::make_pair(model::SKILL_FROST_BOLT, 5));
}

TEST(get_skill_to_learn, for_begin_with_unknown_skiil_from_message) {
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
    EXPECT_EQ(get_skill_to_learn(context, model::_SKILL_UNKNOWN_), model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_1);
}

TEST(get_skill_to_learn, for_begin_with_frost_bolt_from_message_should_return_magical_damage_bonus_passive_1) {
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
    EXPECT_EQ(get_skill_to_learn(context, model::SKILL_FROST_BOLT), model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_1);
}

TEST(get_skill_to_learn, for_wizard_with_staff_damage_bonus_passive_1_and_with_unknown_skiil_from_message) {
    const auto self = wizard_with_skill(model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_1);
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
    EXPECT_EQ(get_skill_to_learn(context, model::_SKILL_UNKNOWN_), model::SKILL_STAFF_DAMAGE_BONUS_AURA_1);
}

TEST(get_skill_to_recommend, for_fire_at_begin_should_return_staff_damage_bonus_passive_1) {
    EXPECT_EQ(get_skill_to_recommend(SELF, Specialization::FIRE), model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_1);
}

} // namespace tests
} // namespace strategy
