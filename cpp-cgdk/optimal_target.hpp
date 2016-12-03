#ifndef STRATEGY_OPTIMAL_TARGET_HPP
#define STRATEGY_OPTIMAL_TARGET_HPP

#include "context.hpp"
#include "helpers.hpp"

#include <iostream>

namespace strategy {

struct GetMaxDamage {
    static const std::vector<model::ActionType> ATTACK_ACTIONS;

    const Context& context;

    double operator ()(const model::Bonus&) const;
    double operator ()(const model::Tree&) const;
    double operator ()(const model::Building& unit) const;
    double operator ()(const model::Minion& unit) const;
    double operator ()(const model::Wizard& unit) const;

    double status_factor(const model::LivingUnit& unit) const;
    double action_factor(const model::Wizard& unit, model::ActionType attack_action) const;
    double action_damage(model::ActionType attack_action) const;
    model::ActionType next_attack_action(const model::Wizard& unit) const;
};

struct IsInMyRange {
    const Context& context;
    const double max_distance;

    template <class T>
    bool operator ()(const T& unit) const {
        return get_position(unit).distance(get_position(context.self())) <= max_distance;
    }
};

struct GetDefenceFactor {
    const Context& context;

    double operator ()(const model::LivingUnit& unit) const;
    double operator ()(const model::Wizard& unit) const;

    double status_factor(const model::LivingUnit& unit) const;
    double skills_factor(const model::Wizard& unit) const;
};

struct GetTargetScore {
    static constexpr const double BUILDING_HIT_PROBABILITY = 1.0;
    static constexpr const double MINION_HIT_PROBABILITY = 0.8;
    static constexpr const double TREE_HIT_PROBABILITY = 1.0;
    static constexpr const double WIZARD_HIT_PROBABILITY = 0.5;

    const Context& context;

    template <class Unit>
    double operator ()(const Unit& unit) const {
        return base(unit) * distance_probability(unit) * angle_probability(unit) * hit_probability(unit);
    }

    double distance_probability(const model::Unit& unit) const;
    double angle_probability(const model::Unit& unit) const;

    double hit_probability(const model::Bonus&) const;
    double hit_probability(const model::Tree&) const;
    double hit_probability(const model::Building& unit) const;
    double hit_probability(const model::Minion& unit) const;
    double hit_probability(const model::Wizard& unit) const;

    double base(const model::Bonus&) const;
    double base(const model::Tree&) const;
    double base(const model::Building& unit) const;
    double base(const model::Minion& unit) const;
    double base(const model::Wizard& unit) const;

    template <class Unit>
    double base_by_damage(const Unit& unit, double damage_score, double elimination_score) const {
        const GetDefenceFactor get_defence_factor {context};
        const auto defence_factor = get_defence_factor(unit);
        const auto max_damage = my_max_damage() * defence_factor;
        damage_score *= defence_factor;
        if (unit.getLife() <= max_damage) {
            return max_damage * (damage_score + elimination_score);
        } else {
            return max_damage * damage_score;
        }
    }

    double my_max_damage() const;
};

struct MakeTargetCandidates {
    template <class Unit>
    using Result = typename std::vector<std::pair<const Unit*, double>>;

    const Context& context;
    const double max_distance;

    template <class Unit>
    Result<Unit> operator ()(const std::unordered_map<UnitId, CachedUnit<Unit>>& units) const {
        const GetTargetScore get_target_score {context};
        Result<Unit> result;
        result.reserve(units.size());
        for (const auto& cached_unit : units) {
            const auto& unit = cached_unit.second.value();
            if (unit.getFaction() != context.self().getFaction() && is_in_my_range(unit)) {
                result.emplace_back(&unit, get_target_score(unit));
            }
        }
        std::sort(result.begin(), result.end(),
            [] (const auto& lhs, const auto& rhs) { return lhs.second > rhs.second; });
        return result;
    }

    bool is_in_my_range(const model::Unit& unit) const;
    bool is_in_my_range(const model::Tree& unit) const;
    bool is_in_my_range(const model::Minion& unit) const;
};

Target get_optimal_target(const Context& context, double max_distance);

}

#endif
