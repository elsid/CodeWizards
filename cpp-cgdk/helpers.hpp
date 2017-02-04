#pragma once

#include "point.hpp"
#include "cache.hpp"
#include "damage.hpp"

#include "model/World.h"
#include "model/ActionType.h"
#include "model/Game.h"

#include <sstream>
#include <tuple>

namespace strategy {

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

static const std::unordered_map<model::SkillType, int> SKILLS_MAGICAL_DAMAGE_ABSORPTION_LEVELS = {
    {model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_1, 1},
    {model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_1, 2},
    {model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_2, 3},
    {model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_2, 4},
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

static constexpr std::array<model::SkillType, model::_ACTION_COUNT_> ACTIONS_SKILLS = {{
    model::_SKILL_UNKNOWN_, // model::ACTION_NONE
    model::_SKILL_UNKNOWN_, // model::ACTION_STAFF
    model::_SKILL_UNKNOWN_, // model::ACTION_MAGIC_MISSILE
    model::SKILL_FROST_BOLT, // model::ACTION_FROST_BOLT
    model::SKILL_FIREBALL, // model::ACTION_FIREBALL
    model::SKILL_HASTE, // model::ACTION_HASTE
    model::SKILL_SHIELD, // model::ACTION_SHIELD
}};

inline bool has_skill(const model::Wizard& unit, model::SkillType skill) {
    return unit.getSkills().end() != std::find(unit.getSkills().begin(), unit.getSkills().end(), skill);
}

inline Point get_position(const model::Unit& unit) {
    return Point(unit.getX(), unit.getY());
}

inline Point get_speed(const model::Unit& unit) {
    return Point(unit.getSpeedX(), unit.getSpeedY());
}

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

inline int get_movement_bonus_level(const model::Unit&) {
    return 0;
}

inline int get_movement_bonus_level(const model::Wizard& unit) {
    return get_skill_bonus_level(unit, SKILLS_MOVEMENT_BONUS_LEVELS);
}

inline int get_staff_damage_bonus_level(const model::Wizard& unit) {
    return get_skill_bonus_level(unit, SKILLS_STAFF_DAMAGE_BONUS_LEVELS);
}

inline int get_magical_damage_bonus_level(const model::Wizard& unit) {
    return get_skill_bonus_level(unit, SKILLS_MAGICAL_DAMAGE_BONUS_LEVELS);
}

inline int get_magical_damage_absorption_level(const model::Wizard& unit) {
    return get_skill_bonus_level(unit, SKILLS_MAGICAL_DAMAGE_ABSORPTION_LEVELS);
}

inline bool is_enemy(const model::Unit& unit, model::Faction my_faction) {
    return unit.getFaction() != my_faction
            && unit.getFaction() != model::FACTION_NEUTRAL
            && unit.getFaction() != model::FACTION_OTHER;
}

inline bool is_friend(const model::Unit& unit, model::Faction my_faction) {
    return unit.getFaction() == my_faction;
}

inline bool is_friend(const model::Wizard& unit, model::Faction my_faction) {
    return unit.getFaction() == my_faction && !unit.isMe();
}

inline bool is_me(const model::Wizard& unit) {
    return unit.isMe();
}

inline bool is_me(const model::Unit&) {
    return false;
}

inline auto find_status(const std::vector<model::Status>& statuses, model::StatusType status) {
    return std::find_if(statuses.begin(), statuses.end(), [&] (const model::Status& v) { return v.getType() == status; });
}

inline int get_hastened_remaining_ticks(const model::LivingUnit& unit) {
    const auto hastend = find_status(unit.getStatuses(), model::STATUS_HASTENED);
    return hastend == unit.getStatuses().end() ? 0 : hastend->getRemainingDurationTicks();
}

inline bool is_with_status(const model::LivingUnit& unit, model::StatusType status) {
    return unit.getStatuses().end() != find_status(unit.getStatuses(), status);
}

inline bool is_empowered(const model::LivingUnit& unit) {
    return is_with_status(unit, model::STATUS_EMPOWERED);
}

inline bool is_shielded(const model::LivingUnit& unit) {
    return is_with_status(unit, model::STATUS_SHIELDED);
}

inline bool is_frozen(const model::LivingUnit& unit) {
    return is_with_status(unit, model::STATUS_FROZEN);
}

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

    inline static const T& get_unit(const std::pair<const UnitId, CachedUnit<T>>& value) {
        return value.second.value();
    }

    inline static const T& get_unit(const T& value) {
        return value;
    }

    inline static const T& get_unit(const T* value) {
        return *value;
    }

    inline static const T& get_unit(const std::pair<const UnitId, T>& value) {
        return value.second;
    }
};

template <class T, class Predicate>
inline std::vector<const T*> filter_units(const std::vector<T>& units, const Predicate& predicate) {
    return FilterUnits<T>::perform(units, predicate);
}

template <class T, class Predicate>
inline std::vector<const T*> filter_units(const std::vector<const T*>& units, const Predicate& predicate) {
    return FilterUnits<T>::perform(units, predicate);
}

template <class T, class Predicate>
inline std::vector<const T*> filter_units(const std::unordered_map<UnitId, T>& units, const Predicate& predicate) {
    return FilterUnits<T>::perform(units, predicate);
}

template <class T, class Predicate>
inline std::vector<const T*> filter_units(const std::unordered_map<UnitId, CachedUnit<T>>& units, const Predicate& predicate) {
    return FilterUnits<T>::perform(units, predicate);
}

template <class T>
inline std::vector<const T*> filter_friends(const std::vector<const T*>& units, model::Faction my_faction) {
    return filter_units(units, [&] (const auto& v) { return is_friend(v, my_faction); });
}

template <class T>
inline std::vector<const T*> filter_friends(const std::vector<T>& units, model::Faction my_faction) {
    return filter_units(units, [&] (const auto& v) { return is_friend(v, my_faction); });
}

inline double line_factor(double value, double zero_at, double one_at) {
    return (value - zero_at) / (one_at - zero_at);
}

inline double bounded_line_factor(double value, double zero_at, double one_at) {
    return std::min(1.0, std::max(0.0, line_factor(value, zero_at, one_at)));
}

template <std::size_t max, std::size_t left, class ... Values>
struct MaxElement {
    template <class Less>
    inline static std::size_t perform(const std::tuple<Values ...>& values, Less less) {
        if (less(std::get<max>(values), std::get<left - 1>(values))) {
            return MaxElement<left - 1, left - 1, Values ...>::perform(values, less);
        } else {
            return MaxElement<max, left - 1, Values ...>::perform(values, less);
        }
    }
};

template <class ... Values>
struct MaxElement<0, 0, Values ...> {
    template <class Less>
    inline static std::size_t perform(const std::tuple<Values ...>&, Less) {
        return 0;
    }
};

template <std::size_t max, class ... Values>
struct MaxElement<max, 0, Values ...> {
    template <class Less>
    inline static std::size_t perform(const std::tuple<Values ...>& values, Less less) {
        return less(std::get<max>(values), std::get<0>(values)) ? 0 : max;
    }
};

template <class Less = std::less<>, class ... Values>
inline std::size_t max_element(const std::tuple<Values ...>& values, Less less = Less()) {
    return MaxElement<sizeof ... (Values) - 1, sizeof ... (Values), Values ...>::perform(values, less);
}

template <std::size_t left, class ... Values>
struct ApplyTo {
    template <class Function>
    inline static auto perform(std::tuple<Values ...>& values, std::size_t index, Function function) {
        if (index == left - 1) {
            return function(std::get<left - 1>(values));
        } else {
            return ApplyTo<left - 1, Values ...>::perform(values, index, function);
        }
    }
};

template <class ... Values>
struct ApplyTo<1, Values ...> {
    template <class Function>
    inline static auto perform(std::tuple<Values ...>& values, std::size_t, Function function) {
        return function(std::get<0>(values));
    }
};

template <class ... Values>
struct ApplyTo<0, Values ...> {};

template <class Function, class ... Values>
inline auto apply_to(std::tuple<Values ...>& values, std::size_t index, Function function) {
    return ApplyTo<sizeof ... (Values), Values ...>::perform(values, index, function);
}

template <std::size_t left, class ... Values>
struct ApplyToConst {
    template <class Function>
    inline static auto perform(const std::tuple<Values ...>& values, std::size_t index, Function function) {
        if (index == left - 1) {
            return function(std::get<left - 1>(values));
        } else {
            return ApplyToConst<left - 1, Values ...>::perform(values, index, function);
        }
    }
};

template <class ... Values>
struct ApplyToConst<1, Values ...> {
    template <class Function>
    inline static auto perform(const std::tuple<Values ...>& values, std::size_t, Function function) {
        return function(std::get<0>(values));
    }
};

template <class ... Values>
struct ApplyToConst<0, Values ...> {};

template <class Function, class ... Values>
inline auto apply_to(const std::tuple<Values ...>& values, std::size_t index, Function function) {
    return ApplyToConst<sizeof ... (Values), Values ...>::perform(values, index, function);
}

inline constexpr model::ProjectileType get_projectile_type_by_action(model::ActionType action) {
    switch (action) {
        case model::ACTION_MAGIC_MISSILE:
            return model::PROJECTILE_MAGIC_MISSILE;
        case model::ACTION_FROST_BOLT:
            return model::PROJECTILE_FROST_BOLT;
        case model::ACTION_FIREBALL:
            return model::PROJECTILE_FIREBALL;
        default:
            return model::_PROJECTILE_UNKNOWN_;
    }
}

inline double get_projectile_radius(model::ProjectileType type, const model::Game& game) {
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

inline double get_projectile_speed(model::ProjectileType type, const model::Game& game) {
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

inline double get_projectile_explosion_radius(model::ProjectileType type, const model::Game& game) {
    switch (type) {
        case model::PROJECTILE_FIREBALL:
            return game.getFireballExplosionMinDamageRange();
        default:
            return 0;
    }
}

inline bool is_owner(const model::Wizard& unit, const model::Projectile& projectile) {
    return projectile.getType() != model::PROJECTILE_DART && unit.getId() == projectile.getOwnerUnitId();
}

inline bool is_owner(const model::Minion& unit, const model::Projectile& projectile) {
    return unit.getType() == model::MINION_FETISH_BLOWDART && projectile.getType() == model::PROJECTILE_DART
            && unit.getId() == projectile.getOwnerUnitId();
}

inline bool is_owner(const model::Unit&, const model::Projectile&) {
    return false;
}

inline double get_attack_range(const model::Minion& unit, const model::Game& game) {
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

inline int get_action_cooldown(model::ActionType action, const model::Wizard& unit, const model::Game& game) {
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

inline double get_status_factor(const model::LivingUnit& unit, const model::Game& game) {
    return is_empowered(unit) * game.getEmpoweredDamageFactor();
}

inline double get_action_factor(model::ActionType action, const model::Wizard& unit, const model::Game& game) {
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

inline Damage get_base_action_damage(model::ActionType action, const model::Game& game) {
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

inline Damage get_action_damage(model::ActionType action, const model::Wizard& unit, const model::Game& game) {
    return (1.0 + get_status_factor(unit, game) + get_action_factor(action, unit, game)) * get_base_action_damage(action, game);
}

}
