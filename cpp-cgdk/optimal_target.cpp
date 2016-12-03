#include "optimal_target.hpp"
#include "optimal_position.hpp"

#include <algorithm>

#ifdef STRATEGY_DEBUG

#include "debug/output.hpp"

#include <iostream>

#endif

namespace strategy {

const std::vector<model::ActionType> GetMaxDamage::ATTACK_ACTIONS = {
    model::ACTION_FIREBALL,
    model::ACTION_FROST_BOLT,
    model::ACTION_MAGIC_MISSILE,
    model::ACTION_STAFF,
};

double GetMaxDamage::operator ()(const model::Bonus&) const {
    return 0.0;
}

double GetMaxDamage::operator ()(const model::Tree&) const {
    return 0.0;
}

double GetMaxDamage::operator ()(const model::Building& unit) const {
    return (1.0 + status_factor(unit)) * unit.getDamage();
}

double GetMaxDamage::operator ()(const model::Minion& unit) const {
    return (1.0 + status_factor(unit)) * unit.getDamage();
}

double GetMaxDamage::operator ()(const model::Wizard& unit) const {
    const auto attack_action = next_attack_action(unit);
    return (1.0 + status_factor(unit) + action_factor(unit, attack_action)) * action_damage(attack_action);
}

double GetMaxDamage::status_factor(const model::LivingUnit& unit) const {
    return is_empowered(unit) * context.game().getEmpoweredDamageFactor();
}

double GetMaxDamage::action_factor(const model::Wizard& unit, model::ActionType attack_action) const {
    switch (attack_action) {
        case model::ACTION_STAFF:
            return get_staff_damage_bonus_level(unit) * context.game().getStaffDamageBonusPerSkillLevel();
        case model::ACTION_MAGIC_MISSILE:
        case model::ACTION_FROST_BOLT:
        case model::ACTION_FIREBALL:
            return get_magical_damage_bonus_level(unit) * context.game().getMagicalDamageBonusPerSkillLevel();
        default:
            return 0;
    }
}

double GetMaxDamage::action_damage(model::ActionType attack_action) const {
    switch (attack_action) {
        case model::ACTION_STAFF:
            return context.game().getStaffDamage();
        case model::ACTION_MAGIC_MISSILE:
            return context.game().getMagicMissileDirectDamage();
        case model::ACTION_FROST_BOLT:
            return context.game().getFrostBoltDirectDamage();
        case model::ACTION_FIREBALL:
            return context.game().getFireballExplosionMaxDamage();
        default:
            return 0;
    }
}

model::ActionType GetMaxDamage::next_attack_action(const model::Wizard& unit) const {
    model::ActionType next_attack_action = model::ACTION_NONE;
    int min_ticks = std::numeric_limits<int>::max();
    for (const auto action : ATTACK_ACTIONS) {
        const auto skill = ACTIONS_SKILLS.at(model::ActionType(action));
        if (skill == model::_SKILL_UNKNOWN_ || has_skill(unit, skill)) {
            const auto ticks = context.self().getRemainingCooldownTicksByAction()[action];
            if (min_ticks > ticks) {
                min_ticks = ticks;
                next_attack_action = action;
            }
        }
    }
    return next_attack_action;
}

Target get_optimal_target(const Context& context, double max_distance) {
    const IsInMyRange is_in_my_range {context, max_distance};
    const GetAttackRange get_attack_range {context};

    const auto is_enemy_and_in_my_range = [&] (const auto& unit) {
        return is_enemy(unit, context.self().getFaction()) && is_in_my_range(unit);
    };

    const auto enemy_wizards = filter_units(context.world().getWizards(), is_enemy_and_in_my_range);
    const auto enemy_minions = filter_units(context.world().getMinions(), is_enemy_and_in_my_range);
    const auto enemy_buildings = filter_units(context.world().getBuildings(), is_enemy_and_in_my_range);

    const auto bonuses = filter_units(context.world().getBonuses(), is_in_my_range);

    const GetMaxDamage get_damage {context};

    const auto get_target_penalty = [&] (const auto& unit) {
        const auto distance = get_position(unit).distance(get_position(context.self()));
        return distance <= 2 * unit.getRadius() + context.self().getVisionRange()
                ? distance * unit.getLife() / get_damage(context.self())
                : distance;
    };

    const auto get_with_min_penalty = [&] (const auto& units) {
        return std::min_element(units.begin(), units.end(),
            [&] (auto lhs, auto rhs) { return get_target_penalty(*lhs) < get_target_penalty(*rhs); });
    };

    const double tree_factor = get_speed(context.self()).norm() < 1 ? 2 : 1;
    const auto trees = filter_units(context.world().getTrees(),
        [&] (const auto& unit) {
            return get_position(context.self()).distance(get_position(unit))
                    < tree_factor * unit.getRadius() + context.game().getStaffRange();
        });
    const auto less_by_distance = [&] (auto lhs, auto rhs) {
        return get_position(*lhs).distance(get_position(context.self())) <
                get_position(*rhs).distance(get_position(context.self()));
    };

    const auto neutral_minions = filter_units(context.world().getMinions(),
          [&] (const auto& unit) {
              return unit.getFaction() == model::FACTION_NEUTRAL &&
                      get_position(context.self()).distance(get_position(unit))
                      < unit.getRadius() + context.self().getRadius() + 10;
          });

    const auto is_in_range_of_my_or_optimal_position = [&] (const auto& unit) {
        const auto optimal_position = get_optimal_position(context, &unit, 2 * context.self().getVisionRange(),
            OPTIMAL_POSITION_INITIAL_POINTS_COUNT, OPTIMAL_POSITION_MINIMIZE_MAX_FUNCTION_CALLS);
        const auto min = std::min(get_position(context.self()).distance(get_position(unit)),
                                  optimal_position.distance(get_position(unit)));
        return min <= get_attack_range(context.self()) + unit.getRadius();
    };

    const model::Wizard* optimal_wizard = nullptr;
    const model::Minion* optimal_minion = nullptr;
    const model::Minion* optimal_neutral_minion = nullptr;
    const model::Bonus* optimal_bonus = nullptr;
    const model::Building* optimal_building = nullptr;
    const model::Tree* optimal_tree = nullptr;

    enum Type {
        OPTIMAL_ENEMY_WIZARD,
        OPTIMAL_ENEMY_MINION,
        OPTIMAL_BONUS,
        OPTIMAL_ENEMY_BUILDING,
        OPTIMAL_TREE,
        OPTIMAL_NEUTRAL_MINION,
        OPTIMAL_COUNT,
    };

    std::vector<double> penalties(OPTIMAL_COUNT, std::numeric_limits<double>::max());

    if (!enemy_wizards.empty()) {
        const auto unit = *get_with_min_penalty(enemy_wizards);
        if (is_in_range_of_my_or_optimal_position(*unit)) {
            optimal_wizard = unit;
            penalties[OPTIMAL_ENEMY_WIZARD] = get_target_penalty(*unit);
        }
    }

    if (!enemy_minions.empty()) {
        const auto unit = *get_with_min_penalty(enemy_minions);
        if (is_in_range_of_my_or_optimal_position(*unit)) {
            optimal_minion = unit;
            penalties[OPTIMAL_ENEMY_MINION] = get_target_penalty(*unit);
        }
    }

    if (!bonuses.empty()) {
        optimal_bonus = *std::min_element(bonuses.begin(), bonuses.end(),
            [&] (auto lhs, auto rhs) {
                return get_position(*lhs).distance(get_position(context.self()))
                        < get_position(*rhs).distance(get_position(context.self()));
            });
        penalties[OPTIMAL_BONUS] = get_position(*optimal_bonus).distance(get_position(context.self()));
    }

    if (!enemy_buildings.empty()) {
        const auto unit = *get_with_min_penalty(enemy_buildings);
        if (is_in_range_of_my_or_optimal_position(*unit)) {
            optimal_building = unit;
            penalties[OPTIMAL_ENEMY_BUILDING] = get_target_penalty(*unit);
        }
    }

    if (!trees.empty()) {
        optimal_tree = *std::min_element(trees.begin(), trees.end(), less_by_distance);
        penalties[OPTIMAL_TREE] = get_position(*optimal_tree).distance(get_position(context.self()));
    }

    if (!neutral_minions.empty()) {
        optimal_neutral_minion = *std::min_element(neutral_minions.begin(), neutral_minions.end(), less_by_distance);
        penalties[OPTIMAL_NEUTRAL_MINION] = get_position(*optimal_neutral_minion).distance(get_position(context.self()));
    }

    const auto optimal = std::min_element(penalties.begin(), penalties.end());

    if (*optimal == std::numeric_limits<double>::max()) {
        return Target();
    }

    switch (Type(optimal - penalties.begin())) {
        case OPTIMAL_ENEMY_WIZARD:
            return get_id(*optimal_wizard);
        case OPTIMAL_ENEMY_MINION:
            return get_id(*optimal_minion);
        case OPTIMAL_BONUS:
            return get_id(*optimal_bonus);
        case OPTIMAL_ENEMY_BUILDING:
            return get_id(*optimal_building);
        case OPTIMAL_TREE:
            return get_id(*optimal_tree);
        case OPTIMAL_NEUTRAL_MINION:
            return get_id(*optimal_neutral_minion);
        case OPTIMAL_COUNT:
            return Target();
    }

    return Target();
}

}
