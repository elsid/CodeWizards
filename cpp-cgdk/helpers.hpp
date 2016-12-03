#pragma once

#include "model/World.h"
#include "model/ActionType.h"

#include "point.hpp"
#include "cache.hpp"

namespace strategy {

static const std::unordered_map<model::SkillType, model::SkillType> SKILLS_DEPENDECIES = {
    {model::SKILL_RANGE_BONUS_PASSIVE_1, model::_SKILL_UNKNOWN_},
    {model::SKILL_RANGE_BONUS_AURA_1, model::SKILL_RANGE_BONUS_PASSIVE_1},
    {model::SKILL_RANGE_BONUS_PASSIVE_2, model::SKILL_RANGE_BONUS_AURA_1},
    {model::SKILL_RANGE_BONUS_AURA_2, model::SKILL_RANGE_BONUS_PASSIVE_2},
    {model::SKILL_ADVANCED_MAGIC_MISSILE, model::SKILL_RANGE_BONUS_AURA_2},

    {model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_1, model::_SKILL_UNKNOWN_},
    {model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_1, model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_1},
    {model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_2, model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_1},
    {model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_2, model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_2},
    {model::SKILL_FROST_BOLT, model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_2},

    {model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_1, model::_SKILL_UNKNOWN_},
    {model::SKILL_STAFF_DAMAGE_BONUS_AURA_1, model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_1},
    {model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_2, model::SKILL_STAFF_DAMAGE_BONUS_AURA_1},
    {model::SKILL_STAFF_DAMAGE_BONUS_AURA_2, model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_2},
    {model::SKILL_FIREBALL, model::SKILL_STAFF_DAMAGE_BONUS_AURA_2},

    {model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_1, model::_SKILL_UNKNOWN_},
    {model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_1, model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_1},
    {model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_2, model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_1},
    {model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_2, model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_2},
    {model::SKILL_HASTE, model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_2},

    {model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_1, model::_SKILL_UNKNOWN_},
    {model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_1, model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_1},
    {model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_2, model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_1},
    {model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_2, model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_2},
    {model::SKILL_SHIELD, model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_2},
};

static const std::unordered_map<model::SkillType, int> SKILLS_MOVEMENT_BONUS_LEVELS = {
    {model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_1, 1},
    {model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_1, 2},
    {model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_2, 3},
    {model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_2, 4},
};

static const std::unordered_map<model::SkillType, int> SKILLS_MAGICAL_DAMAGE_BONUS_LEVELS = {
    {model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_1, 1},
    {model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_1, 2},
    {model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_2, 3},
    {model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_2, 4},
};

static const std::unordered_map<model::SkillType, int> SKILLS_STAFF_DAMAGE_BONUS_LEVELS = {
    {model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_1, 1},
    {model::SKILL_STAFF_DAMAGE_BONUS_AURA_1, 2},
    {model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_2, 3},
    {model::SKILL_STAFF_DAMAGE_BONUS_AURA_2, 4},
};

static const std::unordered_map<model::ActionType, std::string> ACTIONS_NAMES = {
    {model::ACTION_NONE, "NONE"},
    {model::ACTION_STAFF, "STAFF"},
    {model::ACTION_MAGIC_MISSILE, "MAGIC_MISSILE"},
    {model::ACTION_FROST_BOLT, "FROST_BOLT"},
    {model::ACTION_FIREBALL, "FIREBALL"},
    {model::ACTION_HASTE, "HASTE"},
    {model::ACTION_SHIELD, "SHIELD"},
};

static const std::unordered_map<model::ActionType, model::SkillType> ACTIONS_SKILLS = {
    {model::ACTION_STAFF, model::_SKILL_UNKNOWN_},
    {model::ACTION_MAGIC_MISSILE, model::_SKILL_UNKNOWN_},
    {model::ACTION_FROST_BOLT, model::SKILL_FROST_BOLT},
    {model::ACTION_FIREBALL, model::SKILL_FIREBALL},
    {model::ACTION_HASTE, model::SKILL_HASTE},
    {model::ACTION_SHIELD, model::SKILL_SHIELD},
};

bool has_skill(const model::Wizard& unit, model::SkillType skill);
bool can_learn_skill(const model::Wizard& unit, model::SkillType skill);
model::SkillType next_to_learn(const model::Wizard& unit, model::SkillType skill);

Point get_position(const model::Unit& unit);

Point get_speed(const model::Unit& unit);

int get_hastened_remaining_ticks(const model::LivingUnit& unit);

template <class T>
int get_skill_bonus_level(const model::Wizard& unit, const T& skills) {
    int result = 0;
    for (const auto skill : unit.getSkills()) {
        const auto level = skills.find(skill);
        if (level != skills.end()) {
            result = std::max(result, level->second);
        }
    }
    return result;
}

int get_movement_bonus_level(const model::Unit& unit);
int get_movement_bonus_level(const model::Wizard& unit);
int get_staff_damage_bonus_level(const model::Wizard& unit);
int get_magical_damage_bonus_level(const model::Wizard& unit);

double normalize_angle(double value);

bool is_enemy(const model::Unit& unit, model::Faction my_faction);

bool is_friend(const model::Unit& unit, model::Faction my_faction, UnitId my_id);
bool is_friend(const model::Wizard& unit, model::Faction my_faction, UnitId my_id);

bool is_me(const model::Wizard& unit);
bool is_me(const model::Unit&);

std::vector<model::Status>::const_iterator find_status(const std::vector<model::Status>& statuses, model::StatusType status);

bool is_with_status(const model::LivingUnit& unit, model::StatusType status);

bool is_empowered(const model::LivingUnit& unit);

bool is_shielded(const model::LivingUnit& unit);

bool is_enemy(const model::Unit& unit, model::Faction my_faction);

template <class T>
struct FilterUnits {
    template <class Units, class Predicate>
    static std::vector<const T*> perform(const Units& units, const Predicate& predicate) {
        std::vector<const T*> result;
        result.reserve(units.size());
        for (const auto& v : units) {
            if (predicate(get_unit(v))) {
                result.push_back(&get_unit(v));
            }
        }
        return result;
    }

    static const T& get_unit(const std::pair<const UnitId, CachedUnit<T>>& value) {
        return value.second.value();
    }

    static const T& get_unit(const T& value) {
        return value;
    }

    static const T& get_unit(const T* value) {
        return *value;
    }
};

template <class T, class Predicate>
std::vector<const T*> filter_units(const std::vector<T>& units, const Predicate& predicate) {
    return FilterUnits<T>::perform(units, predicate);
}

template <class T, class Predicate>
inline std::vector<const T*> filter_units(const std::vector<const T*>& units, const Predicate& predicate) {
    return FilterUnits<T>::perform(units, predicate);
}

template <class T, class Predicate>
inline std::vector<const T*> filter_units(const std::unordered_map<UnitId, CachedUnit<T>>& units, const Predicate& predicate) {
    return FilterUnits<T>::perform(units, predicate);
}

template <class T>
inline std::vector<const T*> filter_friends(const std::vector<const T*>& units, model::Faction my_faction, UnitId my_id) {
    return filter_units(units, [&] (const auto& v) { return is_friend(v, my_faction, my_id); });
}

template <class T>
inline std::vector<const T*> filter_friends(const std::vector<T>& units, model::Faction my_faction, UnitId my_id) {
    return filter_units(units, [&] (const auto& v) { return is_friend(v, my_faction, my_id); });
}

}
