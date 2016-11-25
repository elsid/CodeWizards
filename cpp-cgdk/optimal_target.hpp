#pragma once

#include "context.hpp"

namespace strategy {

class Target {
public:
    Target() = default;

    Target(const model::Bonus* unit) : bonus_(unit) {}
    Target(const model::Building* unit) : building_(unit) {}
    Target(const model::Minion* unit) : minion_(unit) {}
    Target(const model::Wizard* unit) : wizard_(unit) {}
    Target(const model::Tree* unit) : tree_(unit) {}

    const model::Bonus* bonus() const { return bonus_; }
    const model::Building* building() const { return building_; }
    const model::Minion* minion() const { return minion_; }
    const model::Wizard* wizard() const { return wizard_; }
    const model::Tree* tree() const { return tree_; }

    bool has_value() const {
        return bonus_ || building_ || minion_ || wizard_ || tree_;
    }

private:
    const model::Bonus* bonus_ = nullptr;
    const model::Building* building_ = nullptr;
    const model::Minion* minion_ = nullptr;
    const model::Wizard* wizard_ = nullptr;
    const model::Tree* tree_ = nullptr;
};

Target get_optimal_target(const Context& context, double max_distance);

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

    bool operator ()(const auto& unit) const {
        return get_position(unit).distance(get_position(context.self)) <= max_distance;
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
        return get_factor(unit) * context.game.getMagicMissileDirectDamage();
    }

    double get_factor(const model::LivingUnit& unit) const {
        return 1 + is_empowered(unit) * context.game.getEmpoweredDamageFactor();
    }
};

}
