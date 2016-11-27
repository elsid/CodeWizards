#pragma once

#include "context.hpp"

namespace strategy {

Target get_optimal_target(const Context& context, double max_distance);

std::vector<model::Status>::const_iterator find_status(const std::vector<model::Status>& statuses, model::StatusType status);

bool is_with_status(const model::LivingUnit& unit, model::StatusType status);

bool is_empowered(const model::LivingUnit& unit);

bool is_shielded(const model::LivingUnit& unit);

bool is_enemy(const model::Unit& unit, model::Faction my_faction);

template <class T, class Predicate>
std::vector<const T*> filter_units(const std::vector<T>& units, const Predicate& predicate) {
    std::vector<const T*> result;
    result.reserve(units.size());
    for (const auto& unit : units) {
        if (predicate(unit)) {
            result.push_back(&unit);
        }
    }
    return result;
}

template <class T, class Predicate>
std::vector<const T*> filter_units(const std::vector<const T*>& units, const Predicate& predicate) {
    std::vector<const T*> result;
    result.reserve(units.size());
    for (auto unit : units) {
        if (predicate(*unit)) {
            result.push_back(unit);
        }
    }
    return result;
}

struct IsInMyRange {
    const Context& context;
    const double max_distance;

    template <class T>
    bool operator ()(const T& unit) const {
        return get_position(unit).distance(get_position(context.self())) <= max_distance;
    }
};

struct GetDamage {
    const Context& context;

    double operator ()(const model::Bonus&) const {
        return 0.0;
    }

    double operator ()(const model::Tree&) const {
        return 0.0;
    }

    double operator ()(const model::Building& unit) const {
        return get_factor(unit) * unit.getDamage();
    }

    double operator ()(const model::Minion& unit) const {
        return get_factor(unit) * unit.getDamage();
    }

    double operator ()(const model::Wizard& unit) const {
        return get_factor(unit) * context.game().getMagicMissileDirectDamage();
    }

    double get_factor(const model::LivingUnit& unit) const {
        return 1 + is_empowered(unit) * context.game().getEmpoweredDamageFactor();
    }
};

}
