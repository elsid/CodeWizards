#pragma once

#include "context.hpp"

namespace strategy {

const model::LivingUnit* get_target(const Context& context);

bool is_with_status(const model::LivingUnit& unit, model::StatusType status);

bool is_empowered(const model::LivingUnit& unit);

bool is_shielded(const model::LivingUnit& unit);

bool is_enemy(const model::Unit& unit, model::Faction my_faction);

bool is_friend(const model::Unit& unit, model::Faction my_faction, UnitId);
bool is_friend(const model::Wizard& unit, model::Faction my_faction, UnitId my_id);

template <class T, class Predicate>
std::vector<const T*> filter_units(const std::vector<T>& units, const Predicate& predicate) {
    std::vector<const T*> result;
    for (const auto& unit : units) {
        if (predicate(unit)) {
            result.push_back(&unit);
        }
    }
    return result;
}

template <class T>
std::vector<const T*> filter_enemies(const std::vector<T>& units, model::Faction my_faction) {
    return filter_units(units, [&] (const auto& v) { return is_enemy(v, my_faction); });
}

template <class T>
std::vector<const T*> filter_friends(const std::vector<T>& units, model::Faction my_faction, UnitId my_id) {
    return filter_units(units, [&] (const auto& v) { return is_friend(v, my_faction, my_id); });
}

Point get_optimal_position(const Context& context, const model::LivingUnit* target);

}
