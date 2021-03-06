#pragma once

#include "context.hpp"
#include "helpers.hpp"
#include "target.hpp"
#include "damage.hpp"

#include <iostream>

namespace strategy {

struct GetAttackRange {
    const Context& context;

    double operator ()(const model::Unit&, double distance) const;
    double operator ()(const model::Building& unit, double distance) const;
    double operator ()(const model::Minion& unit, double distance) const;
    double operator ()(const model::Wizard& unit, double distance) const;
    double operator ()(const model::Wizard& unit, model::ActionType action) const;
};

struct GetMaxDamage {
    static const std::array<model::ActionType, 4> ATTACK_ACTIONS;

    const Context& context;

    Damage operator ()(const model::Bonus&, double distance) const;
    Damage operator ()(const model::Tree&, double distance) const;
    Damage operator ()(const model::Building& unit, double distance) const;
    Damage operator ()(const model::Minion& unit, double distance) const;
    Damage operator ()(const model::Wizard& unit, double distance) const;

    double status_factor(const model::LivingUnit& unit) const;
    double action_factor(const model::Wizard& unit, model::ActionType attack_action) const;
    Damage action_damage(model::ActionType attack_action, const model::Wizard& unit) const;
    Damage action_damage(model::ActionType attack_action) const;
    Tick action_cooldown(model::ActionType attack_action, const model::Wizard& unit) const;
    std::pair<model::ActionType, Tick> next_attack_action(const model::Wizard& unit, double distance) const;
};

struct IsInMyRange {
    const Context& context;
    const double max_distance;

    template <class T>
    bool operator ()(const T& unit) const {
        return get_position(unit).distance(get_position(context.self())) - unit.getRadius() <= max_distance;
    }
};

struct ReduceDamage {
    const Context& context;

    Damage operator ()(const model::LivingUnit& unit, const Damage& damage) const;
    Damage operator ()(const model::Wizard& unit, const Damage& damage) const;

    double get_status_factor(const model::LivingUnit& unit) const;
    double get_skills_reduction(const model::Wizard& unit) const;
};

struct GetLifeRegeneration {
    const Context& context;

    double operator ()(const model::LivingUnit& unit) const;
    double operator ()(const model::Wizard& unit) const;
};

struct GetUnitAttackAbility {
    const Context& context;

    template <class Unit>
    double operator ()(const Unit&) const {
        return 0;
    }

    double operator ()(const model::Building& unit) const;
    double operator ()(const model::Minion& unit) const;
    double operator ()(const model::Wizard& unit) const;
};

struct GetTargetScore {
    static constexpr double BUILDING_HIT_PROBABILITY = 1.0;
    static constexpr double MINION_HIT_PROBABILITY = 1.0;
    static constexpr double TREE_HIT_PROBABILITY = 1.0;
    static constexpr double WIZARD_HIT_PROBABILITY = 0.5;

    const Context& context;

    template <class Unit>
    double operator ()(const Unit& unit) const {
        const auto base = get_base(unit);
        const auto distance_probability = get_distance_probability(unit);
        const auto angle_probability = get_angle_probability(unit);
        const auto hit_probability = get_hit_probability(unit);
        return base * distance_probability * angle_probability * hit_probability;
    }

    double get_distance_probability(const model::Unit& unit) const;
    double get_angle_probability(const model::Unit& unit) const;

    double get_hit_probability(const model::Bonus&) const;
    double get_hit_probability(const model::Tree&) const;
    double get_hit_probability(const model::Building& unit) const;
    double get_hit_probability(const model::Minion& unit) const;
    double get_hit_probability(const model::Wizard& unit) const;

    double get_hit_probability_by_status(const model::LivingUnit& unit, double base) const;

    double get_base(const model::Bonus&) const;
    double get_base(const model::Tree&) const;
    double get_base(const model::Building& unit) const;
    double get_base(const model::Minion& unit) const;
    double get_base(const model::Wizard& unit) const;

    template <class Unit>
    double get_base_by_damage(const Unit& unit, double damage_score, double elimination_score_factor) const {
        const ReduceDamage reduce_damage {context};
        const GetLifeRegeneration get_life_regeneration {context};
        const auto distance = get_position(context.self()).distance(get_position(unit));
        const auto my_max_damage = get_my_max_damage(distance);
        const auto reduced_my_max_damage = reduce_damage(unit, my_max_damage);
        const auto life_regeneration = get_life_regeneration(unit);
        const auto max_damage = reduced_my_max_damage.sum() - life_regeneration * context.game().getWizardActionCooldownTicks();
        if (unit.getLife() <= max_damage) {
            return max_damage * damage_score + elimination_score_factor * unit.getMaxLife();
        } else {
            return max_damage * damage_score;
        }
    }

    Damage get_my_max_damage(double distance) const;
};

struct MakeTargetCandidates {
    template <class Unit>
    using Result = typename std::vector<std::pair<const Unit*, double>>;

    const Context& context;
    const double max_distance;

    template <class Unit>
    bool is_candidate(const std::pair<const UnitId, CachedUnit<Unit>>& cached_unit) const {
        const auto& unit = cached_unit.second.value();
        return unit.getFaction() != context.self().getFaction() && is_in_my_range(cached_unit.second);
    }

    template <class Unit>
    Result<Unit> operator ()(const std::unordered_map<UnitId, CachedUnit<Unit>>& units) const {
        const GetTargetScore get_target_score {context};
        Result<Unit> result;
        result.reserve(units.size());
        for (const auto& cached_unit : units) {
            if (is_candidate(cached_unit)) {
                const auto& unit = cached_unit.second.value();
                if (const auto score = get_target_score(unit)) {
                    result.emplace_back(&unit, score);
                }
            }
        }
        std::sort(result.begin(), result.end(),
            [] (const auto& lhs, const auto& rhs) { return lhs.second > rhs.second; });
        return result;
    }

    template <class Unit>
    bool is_in_my_range(const CachedUnit<Unit>& unit) const {
        return is_in_my_range(unit.value());
    }

    bool is_in_my_range(const model::CircularUnit& unit) const;
    bool is_in_my_range(const model::Tree& unit) const;
    bool is_in_my_range(const CachedUnit<model::Minion>& unit) const;
};

double get_max_distance_for_tree_candidate(const Context& context);
double get_max_distance_for_neutral_minion_candidate(const Context& context);
double get_max_distance_for_unit_candidate(const Context& context);
double get_max_distance_for_optimal_position(const Context& context);

bool has_candidates(const Context& context, double max_distance);
Target get_optimal_target(const Context& context, double max_distance);

}
