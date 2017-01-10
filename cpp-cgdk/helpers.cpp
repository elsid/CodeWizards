#include "helpers.hpp"

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

double normalize_angle(double value) {
    if (value > M_PI) {
        return value - std::round(value * 0.5 * M_1_PI) * 2.0 * M_PI;
    }
    if (value < -M_PI) {
        return value + std::round(std::abs(value) * 0.5 * M_1_PI) * 2.0 * M_PI;
    }
    return value;
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

}
