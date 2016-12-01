#ifndef STRATEGY_OPTIMAL_TARGET_HPP
#define STRATEGY_OPTIMAL_TARGET_HPP

#include "context.hpp"
#include "helpers.hpp"

namespace strategy {

struct GetDamage {
    const Context& context;

    double operator ()(const model::Bonus&) const;
    double operator ()(const model::Tree&) const;
    double operator ()(const model::Building& unit) const;
    double operator ()(const model::Minion& unit) const;
    double operator ()(const model::Wizard& unit) const;
    double get_statuses_factor(const model::LivingUnit& unit) const;
    double get_skills_factor(const model::Wizard& unit) const;
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
