#include "optimal_target.hpp"
#include "optimal_position.hpp"

#include <algorithm>
#include <cassert>

#ifdef STRATEGY_DEBUG

#include "debug/output.hpp"

#include <iostream>

#endif

namespace strategy {

const std::array<model::ActionType, 4> GetMaxDamage::ATTACK_ACTIONS = {{
    model::ACTION_FIREBALL,
    model::ACTION_FROST_BOLT,
    model::ACTION_MAGIC_MISSILE,
    model::ACTION_STAFF,
}};

double GetAttackRange::operator ()(const model::Unit&, double) const {
    return 0.0;
}

double GetAttackRange::operator ()(const model::Building& unit, double) const {
    return unit.getAttackRange();
}

double GetAttackRange::operator ()(const model::Minion& unit, double) const {
    switch (unit.getType()) {
        case model::_MINION_UNKNOWN_:
            break;
        case model::MINION_ORC_WOODCUTTER:
            return context.game().getOrcWoodcutterAttackRange();
        case model::MINION_FETISH_BLOWDART:
            return context.game().getFetishBlowdartAttackRange() + context.game().getDartRadius();
        case model::_MINION_COUNT_:
            break;
    }
    std::ostringstream error;
    error << "Invalid minion type: " << int(unit.getType())
          << " in " << __PRETTY_FUNCTION__ << " at " << __FILE__ << ":" << __LINE__;
    throw std::logic_error(error.str());
}

double GetAttackRange::operator ()(const model::Wizard& unit, double distance) const {
    const GetMaxDamage get_max_damage {context};
    return (*this)(unit, get_max_damage.next_attack_action(unit, distance));
}

double GetAttackRange::operator ()(const model::Wizard& unit, model::ActionType action) const {
    switch (action) {
        case model::ACTION_STAFF:
            return context.game().getStaffRange();
        case model::ACTION_MAGIC_MISSILE:
            return unit.getCastRange() + context.game().getMagicMissileRadius();
        case model::ACTION_FROST_BOLT:
            return unit.getCastRange() + context.game().getFrostBoltRadius();
        case model::ACTION_FIREBALL:
            return unit.getCastRange() + std::max(context.game().getFireballRadius(),
                                                  context.game().getFireballExplosionMaxDamageRange());
        default:
            return 0;
    }
}

double GetMaxDamage::operator ()(const model::Bonus&, double) const {
    return 0.0;
}

double GetMaxDamage::operator ()(const model::Tree&, double) const {
    return 0.0;
}

double GetMaxDamage::operator ()(const model::Building& unit, double) const {
    return (1.0 + status_factor(unit)) * unit.getDamage();
}

double GetMaxDamage::operator ()(const model::Minion& unit, double) const {
    return (1.0 + status_factor(unit)) * unit.getDamage();
}

double GetMaxDamage::operator ()(const model::Wizard& unit, double distance) const {
    const auto attack_action = next_attack_action(unit, distance);
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
            return context.game().getFireballExplosionMaxDamage() + context.game().getBurningSummaryDamage();
        default:
            return 0;
    }
}

model::ActionType GetMaxDamage::next_attack_action(const model::Wizard& unit, double distance) const {
    const GetAttackRange get_attack_range {context};
    model::ActionType next_attack_action = model::ACTION_NONE;
    int min_ticks = std::numeric_limits<int>::max();
    for (const auto action : ATTACK_ACTIONS) {
        const auto skill = ACTIONS_SKILLS.at(model::ActionType(action));
        if (skill == model::_SKILL_UNKNOWN_ || has_skill(unit, skill)) {
            const auto ticks = unit.getRemainingCooldownTicksByAction()[action];
            if (min_ticks > ticks && distance <= get_attack_range(unit, model::ActionType(action))) {
                min_ticks = ticks;
                next_attack_action = action;
            }
        }
    }
    if (next_attack_action != model::ACTION_NONE) {
        return next_attack_action;
    }
    double max_range = 0;
    for (const auto action : ATTACK_ACTIONS) {
        const auto skill = ACTIONS_SKILLS.at(model::ActionType(action));
        if (skill == model::_SKILL_UNKNOWN_ || has_skill(unit, skill)) {
            const auto range = get_attack_range(unit, model::ActionType(action));
            if (max_range < range) {
                max_range = range;
                next_attack_action = action;
            }
        }
    }
    return next_attack_action;
}

double GetDefenceFactor::operator ()(const model::LivingUnit& unit) const {
    return 1.0 - status_factor(unit);
}

double GetDefenceFactor::operator ()(const model::Wizard& unit) const {
    return 1.0 - status_factor(unit) - skills_factor(unit);
}

double GetDefenceFactor::status_factor(const model::LivingUnit& unit) const {
    return is_shielded(unit) * context.game().getShieldedDirectDamageAbsorptionFactor();
}

double GetDefenceFactor::skills_factor(const model::Wizard& unit) const {
    return get_magical_damage_absorption_level(unit) * context.game().getMagicalDamageAbsorptionPerSkillLevel();
}

double GetLifeRegeneration::operator ()(const model::LivingUnit&) const {
    return 0;
}

double GetLifeRegeneration::operator ()(const model::Wizard& unit) const {
    return context.game().getWizardBaseLifeRegeneration()
            + unit.getLevel() * context.game().getWizardLifeRegenerationGrowthPerLevel();
}

double GetTargetScore::distance_probability(const model::Unit& unit) const {
    const auto distance = get_position(context.self()).distance(get_position(unit));
    return line_factor(distance, 2 * context.self().getCastRange(), 0);
}

double GetTargetScore::angle_probability(const model::Unit& unit) const {
    const auto direction = get_position(unit) - get_position(context.self());
    const auto angle = normalize_angle(direction.absolute_rotation() - context.self().getAngle());
    if (angle >= 0) {
        return line_factor(angle, 2.0 * M_PI, 0);
    } else {
        return line_factor(angle, -2.0 * M_PI, 0);
    }
}

double GetTargetScore::hit_probability(const model::Bonus&) const {
    return 1.0;
}

double GetTargetScore::hit_probability(const model::Tree&) const {
    return TREE_HIT_PROBABILITY;
}

double GetTargetScore::hit_probability(const model::Building&) const {
    return BUILDING_HIT_PROBABILITY;
}

double GetTargetScore::hit_probability(const model::Minion&) const {
    return MINION_HIT_PROBABILITY;
}

double GetTargetScore::hit_probability(const model::Wizard&) const {
    // TODO: use hastened status and movement bonus skills
    return WIZARD_HIT_PROBABILITY;
}

double GetTargetScore::base(const model::Bonus&) const {
    return context.game().getBonusScoreAmount();
}

double GetTargetScore::base(const model::Tree&) const {
    return 2;
}

double GetTargetScore::base(const model::Building& unit) const {
    const auto by_damage = base_by_damage(unit, context.game().getBuildingDamageScoreFactor(),
                                          context.game().getBuildingEliminationScoreFactor());
    if (unit.getType() == model::BUILDING_FACTION_BASE) {
        return context.game().getVictoryScore() + by_damage;
    } else {
        return by_damage;
    }
}

double GetTargetScore::base(const model::Minion& unit) const {
    if (unit.getFaction() == model::FACTION_NEUTRAL) {
        return 1;
    } else {
        return base_by_damage(unit, context.game().getMinionDamageScoreFactor(),
                              context.game().getMinionEliminationScoreFactor()) + 1;
    }
}

double GetTargetScore::base(const model::Wizard& unit) const {
    return base_by_damage(unit, context.game().getWizardDamageScoreFactor(),
                          context.game().getWizardEliminationScoreFactor());
}

double GetTargetScore::my_max_damage(double distance) const {
    const GetMaxDamage get_max_damage {context};
    return get_max_damage(context.self(), distance);
}

bool MakeTargetCandidates::is_in_my_range(const model::Unit& unit) const {
    return get_position(unit).distance(get_position(context.self())) <= max_distance;
}

bool MakeTargetCandidates::is_in_my_range(const model::Tree& unit) const {
    const auto factor = get_speed(context.self()).norm() < 1 ? 2 : 1;
    return get_position(unit).distance(get_position(context.self())) <= factor * unit.getRadius() + context.game().getStaffRange();
}

bool MakeTargetCandidates::is_in_my_range(const model::Minion& unit) const {
    if (unit.getFaction() == model::FACTION_NEUTRAL) {
        return get_position(unit).distance(get_position(context.self())) <= unit.getRadius() + context.game().getStaffRange();
    } else {
        return is_in_my_range(static_cast<const model::Unit&>(unit));
    }
}

struct GetOptimalTarget {
    template <class Unit>
    using Iterator = typename MakeTargetCandidates::Result<Unit>::const_iterator;

    using Iterators = std::tuple<
        Iterator<model::Bonus>,
        Iterator<model::Building>,
        Iterator<model::Minion>,
        Iterator<model::Tree>,
        Iterator<model::Wizard>
    >;

    struct LessByScore {
        const Iterators& ends;

        template <class Lhs, class Rhs>
        bool operator ()(Lhs lhs, Rhs rhs) const {
            if (lhs != std::get<Lhs>(ends) && rhs != std::get<Rhs>(ends)) {
                return lhs->second < rhs->second;
            } else {
                return lhs == std::get<Lhs>(ends) && rhs != std::get<Rhs>(ends);
            }
        }
    };

    const Context& context;

    Target operator ()(const Iterators& begins, const Iterators& ends) const {
        const GetAttackRange get_attack_range {context};
        const LessByScore less_by_score {ends};

        Target result;

        for (auto iterators = begins; iterators != ends;) {
            context.check_timeout(__PRETTY_FUNCTION__, __FILE__, __LINE__);

            const auto max = max_element(iterators, less_by_score);

            const auto set_result = [&] (auto candidate) {
                const auto optimal_position = get_optimal_position(context, candidate->first, 2 * context.self().getVisionRange(),
                    OPTIMAL_POSITION_INITIAL_POINTS_COUNT, OPTIMAL_POSITION_MINIMIZE_MAX_FUNCTION_CALLS);
                const auto path = get_optimal_path(context, optimal_position, OPTIMAL_PATH_STEP_SIZE, OPTIMAL_PATH_MAX_TICKS);
                auto min_distance = get_position(context.self()).distance(get_position(*candidate->first));
                if (!path.empty()) {
                    min_distance = std::min(min_distance, path.back().distance(get_position(*candidate->first)));
                }
                if (min_distance <= get_attack_range(context.self(), min_distance) + candidate->first->getRadius()) {
                    result = get_id(*candidate->first);
                }
            };

            apply_to(iterators, max, set_result);

            if (result.is_some()) {
                break;
            }

            apply_to(iterators, max, [] (auto& it) { ++it; });
        }

        if (!result.is_some()) {
            double min_distance = std::numeric_limits<double>::max();

            update_target<model::Bonus>(begins, ends, result, min_distance);
            update_target<model::Building>(begins, ends, result, min_distance);
            update_target<model::Minion>(begins, ends, result, min_distance);
            update_target<model::Tree>(begins, ends, result, min_distance);
            update_target<model::Wizard>(begins, ends, result, min_distance);
        }

        return result;
    }

    template <class Unit>
    void update_target(const Iterators& begins, const Iterators& ends, Target& target, double& min_distance) const {
        if (std::get<Iterator<Unit>>(begins) == std::get<Iterator<Unit>>(ends)) {
            return;
        }

        const auto less_by_distance = [&] (const auto& lhs, const auto& rhs) {
            return get_position(*lhs.first).distance(get_position(context.self()))
                    < get_position(*rhs.first).distance(get_position(context.self()));
        };

        const auto unit = std::min_element(std::get<Iterator<Unit>>(begins), std::get<Iterator<Unit>>(ends), less_by_distance);
        const auto distance = get_position(*unit->first).distance(get_position(context.self()));

        if (min_distance > distance) {
            min_distance = distance;
            target = get_id(*unit->first);
        }
    }
};

Target get_optimal_target(const Context& context, double max_distance) {
    const MakeTargetCandidates make_target_candidates {context, max_distance};

    const auto bonuses_candidates = make_target_candidates(get_units<model::Bonus>(context.cache()));
    const auto buildings_candidates = make_target_candidates(get_units<model::Building>(context.cache()));
    const auto minions_candidates = make_target_candidates(get_units<model::Minion>(context.cache()));
    const auto trees_candidates = make_target_candidates(get_units<model::Tree>(context.cache()));
    const auto wizards_candidates = make_target_candidates(get_units<model::Wizard>(context.cache()));

    const GetOptimalTarget impl {context};

    const GetOptimalTarget::Iterators begins(
        bonuses_candidates.begin(),
        buildings_candidates.begin(),
        minions_candidates.begin(),
        trees_candidates.begin(),
        wizards_candidates.begin()
    );

    const GetOptimalTarget::Iterators ends(
        bonuses_candidates.end(),
        buildings_candidates.end(),
        minions_candidates.end(),
        trees_candidates.end(),
        wizards_candidates.end()
    );

    return impl(begins, ends);
}

}
