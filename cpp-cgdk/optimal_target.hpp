#ifndef STRATEGY_OPTIMAL_TARGET_HPP
#define STRATEGY_OPTIMAL_TARGET_HPP

#include "context.hpp"
#include "helpers.hpp"

namespace strategy {

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

struct IsInMyRange {
    const Context& context;
    const double max_distance;

    template <class T>
    bool operator ()(const T& unit) const {
        return get_position(unit).distance(get_position(context.self())) <= max_distance;
    }
};

Target get_optimal_target(const Context& context, double max_distance);

}

#endif
