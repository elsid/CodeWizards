#pragma once

#include "model/World.h"

#include "point.hpp"
#include "cache.hpp"

namespace strategy {

bool has_skill(const model::Wizard& unit, model::SkillType skill);

Point get_position(const model::Unit& unit);

Point get_speed(const model::Unit& unit);

int get_hastened_remaining_ticks(const model::LivingUnit& unit);

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
