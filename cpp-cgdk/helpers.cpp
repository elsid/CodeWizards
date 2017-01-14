#include "helpers.hpp"

#include <sstream>

namespace strategy {

bool has_skill(const model::Wizard& unit, model::SkillType skill) {
    return unit.getSkills().end() != std::find(unit.getSkills().begin(), unit.getSkills().end(), skill);
}

Point get_position(const model::Unit& unit) {
    return Point(unit.getX(), unit.getY());
}

Point get_speed(const model::Unit& unit) {
    return Point(unit.getSpeedX(), unit.getSpeedY());
}

int get_hastened_remaining_ticks(const model::LivingUnit& unit) {
    const auto hastend = find_status(unit.getStatuses(), model::STATUS_HASTENED);
    return hastend == unit.getStatuses().end() ? 0 : hastend->getRemainingDurationTicks();
}

int get_movement_bonus_level(const model::Unit&) {
    return 0;
}

int get_movement_bonus_level(const model::Wizard& unit) {
    return get_skill_bonus_level(unit, SKILLS_MOVEMENT_BONUS_LEVELS);
}

int get_staff_damage_bonus_level(const model::Wizard& unit) {
    return get_skill_bonus_level(unit, SKILLS_STAFF_DAMAGE_BONUS_LEVELS);
}

int get_magical_damage_bonus_level(const model::Wizard& unit) {
    return get_skill_bonus_level(unit, SKILLS_MAGICAL_DAMAGE_BONUS_LEVELS);
}

int get_magical_damage_absorption_level(const model::Wizard& unit) {
    return get_skill_bonus_level(unit, SKILLS_MAGICAL_DAMAGE_ABSORPTION_LEVELS);
}

bool is_friend(const model::Unit& unit, model::Faction my_faction) {
    return unit.getFaction() == my_faction;
}

bool is_friend(const model::Wizard& unit, model::Faction my_faction) {
    return unit.getFaction() == my_faction && !unit.isMe();
}

bool is_me(const model::Wizard& unit) {
    return unit.isMe();
}

bool is_me(const model::Unit&) {
    return false;
}

std::vector<model::Status>::const_iterator find_status(const std::vector<model::Status>& statuses, model::StatusType status) {
    return std::find_if(statuses.begin(), statuses.end(),
                        [&] (const model::Status& v) { return v.getType() == status; });
}

bool is_with_status(const model::LivingUnit& unit, model::StatusType status) {
    return unit.getStatuses().end() != find_status(unit.getStatuses(), status);
}

bool is_empowered(const model::LivingUnit& unit) {
    return is_with_status(unit, model::STATUS_EMPOWERED);
}

bool is_shielded(const model::LivingUnit& unit) {
    return is_with_status(unit, model::STATUS_SHIELDED);
}

bool is_frozen(const model::LivingUnit& unit) {
    return is_with_status(unit, model::STATUS_FROZEN);
}

bool is_enemy(const model::Unit& unit, model::Faction my_faction) {
    return unit.getFaction() != my_faction
            && unit.getFaction() != model::FACTION_NEUTRAL
            && unit.getFaction() != model::FACTION_OTHER;
}

double bounded_line_factor(double value, double zero_at, double one_at) {
    return std::min(1.0, std::max(0.0, line_factor(value, zero_at, one_at)));
}

double line_factor(double value, double zero_at, double one_at) {
    return (value - zero_at) / (one_at - zero_at);
}

double get_projectile_radius(model::ProjectileType type, const model::Game& game) {
    switch (type) {
        case model::PROJECTILE_MAGIC_MISSILE:
            return game.getMagicMissileRadius();
        case model::PROJECTILE_FROST_BOLT:
            return game.getFrostBoltRadius();
        case model::PROJECTILE_FIREBALL:
            return game.getFireballRadius();
        default:
            return 0;
    }
}

double get_projectile_speed(model::ProjectileType type, const model::Game& game) {
    switch (type) {
        case model::PROJECTILE_MAGIC_MISSILE:
            return game.getMagicMissileSpeed();
        case model::PROJECTILE_FROST_BOLT:
            return game.getFrostBoltSpeed();
        case model::PROJECTILE_FIREBALL:
            return game.getFireballSpeed();
        default:
            return 0;
    }
}

double get_projectile_explosion_radius(model::ProjectileType type, const model::Game& game) {
    switch (type) {
        case model::PROJECTILE_FIREBALL:
            return game.getFireballExplosionMinDamageRange();
        default:
            return 0;
    }
}

bool is_owner(const model::Wizard& unit, const model::Projectile& projectile) {
    return projectile.getType() != model::PROJECTILE_DART && unit.getId() == projectile.getOwnerUnitId();
}

bool is_owner(const model::Minion& unit, const model::Projectile& projectile) {
    return unit.getType() == model::MINION_FETISH_BLOWDART && projectile.getType() == model::PROJECTILE_DART
            && unit.getId() == projectile.getOwnerUnitId();
}

bool is_owner(const model::Unit&, const model::Projectile&) {
    return false;
}

double get_attack_range(const model::Minion& unit, const model::Game& game) {
    switch (unit.getType()) {
        case model::_MINION_UNKNOWN_:
            break;
        case model::MINION_ORC_WOODCUTTER:
            return game.getOrcWoodcutterAttackRange();
        case model::MINION_FETISH_BLOWDART:
            return game.getFetishBlowdartAttackRange() + game.getDartRadius();
        case model::_MINION_COUNT_:
            break;
    }
    std::ostringstream error;
    error << "Invalid minion type: " << int(unit.getType())
          << " in " << __PRETTY_FUNCTION__ << " at " << __FILE__ << ":" << __LINE__;
    throw std::logic_error(error.str());
}

int get_action_cooldown(model::ActionType action, const model::Wizard& unit, const model::Game& game) {
    switch (action) {
        case model::ACTION_STAFF:
            return game.getStaffCooldownTicks();
        case model::ACTION_MAGIC_MISSILE:
            return has_skill(unit, model::SKILL_ADVANCED_MAGIC_MISSILE) ? 0 : game.getMagicMissileCooldownTicks();
        case model::ACTION_FROST_BOLT:
            return game.getFrostBoltCooldownTicks();
        case model::ACTION_FIREBALL:
            return game.getFireballCooldownTicks();
        case model::ACTION_SHIELD:
            return game.getShieldCooldownTicks();
        case model::ACTION_HASTE:
            return game.getHasteCooldownTicks();
        default:
            return 0;
    }
}

Damage get_action_damage(model::ActionType action, const model::Wizard& unit, const model::Game& game) {
    return (1.0 + get_status_factor(unit, game) + get_action_factor(action, unit, game)) * get_base_action_damage(action, game);
}

Damage get_base_action_damage(model::ActionType action, const model::Game& game) {
    switch (action) {
        case model::ACTION_STAFF:
            return Damage::Physic {double(game.getStaffDamage())};
        case model::ACTION_MAGIC_MISSILE:
            return Damage::Magic {double(game.getMagicMissileDirectDamage())};
        case model::ACTION_FROST_BOLT:
            return Damage::Magic {double(game.getFrostBoltDirectDamage())};
        case model::ACTION_FIREBALL:
            return Damage::Magic {double(game.getFireballExplosionMaxDamage() + game.getBurningSummaryDamage())};
        default:
            return Damage();
    }
}

double get_status_factor(const model::LivingUnit& unit, const model::Game& game) {
    return is_empowered(unit) * game.getEmpoweredDamageFactor();
}

double get_action_factor(model::ActionType action, const model::Wizard& unit, const model::Game& game) {
    switch (action) {
        case model::ACTION_STAFF:
            return get_staff_damage_bonus_level(unit) * game.getStaffDamageBonusPerSkillLevel();
        case model::ACTION_MAGIC_MISSILE:
        case model::ACTION_FROST_BOLT:
        case model::ACTION_FIREBALL:
            return get_magical_damage_bonus_level(unit) * game.getMagicalDamageBonusPerSkillLevel();
        default:
            return 0;
    }
}

}
