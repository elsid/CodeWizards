#include "optimal_target.hpp"
#include "optimal_position.hpp"
#include "optimal_destination.hpp"

#include <algorithm>

#ifdef ELSID_STRATEGY_DEBUG

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
    return (*this)(unit, get_max_damage.next_attack_action(unit, distance).first);
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
    const auto attack_action = next_attack_action(unit, distance).first;
    return action_damage(attack_action, unit);
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

double GetMaxDamage::action_damage(model::ActionType attack_action, const model::Wizard& unit) const {
    return (1.0 + status_factor(unit) + action_factor(unit, attack_action)) * action_damage(attack_action);
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

std::pair<model::ActionType, Tick> GetMaxDamage::next_attack_action(const model::Wizard& unit, double distance) const {
    const GetAttackRange get_attack_range {context};
    model::ActionType next_attack_action = model::ACTION_NONE;
    Tick min_ticks = std::numeric_limits<int>::max();
    double max_damage = 0;
    for (const auto action : ATTACK_ACTIONS) {
        const auto skill = ACTIONS_SKILLS.at(model::ActionType(action));
        if (skill == model::_SKILL_UNKNOWN_ || has_skill(unit, skill)) {
            const auto ticks = std::max(unit.getRemainingCooldownTicksByAction()[action], unit.getRemainingActionCooldownTicks());
            const auto damage = action_damage(action, unit);
            if ((min_ticks > ticks || (min_ticks == ticks && max_damage < damage)) && distance <= get_attack_range(unit, model::ActionType(action))) {
                min_ticks = ticks;
                max_damage = damage;
                next_attack_action = action;
            }
        }
    }
    if (next_attack_action != model::ACTION_NONE) {
        return {next_attack_action, min_ticks};
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
    return {next_attack_action, std::max(unit.getRemainingCooldownTicksByAction()[next_attack_action],
                                         unit.getRemainingActionCooldownTicks())};
}

Tick GetMaxDamage::action_cooldown(model::ActionType attack_action, const model::Wizard& unit) const {
    switch (attack_action) {
        case model::ACTION_STAFF:
            return context.game().getStaffCooldownTicks();
        case model::ACTION_MAGIC_MISSILE:
            return has_skill(unit, model::SKILL_ADVANCED_MAGIC_MISSILE) ? 0 : context.game().getMagicMissileCooldownTicks();
        case model::ACTION_FROST_BOLT:
            return context.game().getFrostBoltCooldownTicks();
        case model::ACTION_FIREBALL:
            return context.game().getFireballCooldownTicks();
        default:
            return 0;
    }
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
    return line_factor(distance, 2 * get_max_distance_for_unit_candidate(context), 0);
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
    return 0.3;
}

double GetTargetScore::base(const model::Tree&) const {
    return 0.2;
}

double GetTargetScore::base(const model::Building& unit) const {
    const TowersOrder tower_order(context.world(), context.self().getFaction());
    bool immortal = true;
    double add = 0;

    const auto is_tower_exists = [&] (const auto& position) {
        const auto& buildings = get_units<model::Building>(context.cache());
        return buildings.end() != std::find_if(buildings.begin(), buildings.end(),
            [&] (const auto& v) { return get_position(v.second.value()).distance(position) < 100; });
    };

    if (unit.getType() == model::BUILDING_FACTION_BASE) {
        const auto lanes = {model::LANE_TOP, model::LANE_MIDDLE, model::LANE_BOTTOM};
        immortal = lanes.end() == std::find_if_not(lanes.begin(), lanes.end(),
            [&] (auto lane) { return is_tower_exists(tower_order.get_enemy_tower(lane, TowerNumber::FIRST)); });
        if (!immortal) {
            add += context.game().getVictoryScore();
        }
    } else {
        const auto lane = tower_order.get_lane(unit);
        const auto second = tower_order.get_enemy_tower(lane, TowerNumber::SECOND);
        if (get_position(unit).distance(second) < 100) {
            immortal = false;
        } else {
            immortal = is_tower_exists(second);
        }
    }

    return immortal ? 0 : add + base_by_damage(unit, context.game().getBuildingDamageScoreFactor(),
                                               context.game().getBuildingEliminationScoreFactor());
}

double GetTargetScore::base(const model::Minion& unit) const {
    if (unit.getFaction() == model::FACTION_NEUTRAL) {
        return 0.1;
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

bool MakeTargetCandidates::is_in_my_range(const model::CircularUnit& unit) const {
    const auto distance = get_position(unit).distance(get_position(context.self()));
    return distance <= max_distance + unit.getRadius();
}

bool MakeTargetCandidates::is_in_my_range(const model::Tree& unit) const {
    const auto distance = get_position(unit).distance(get_position(context.self()));
    return distance <= std::min(max_distance, get_max_distance_for_tree_candidate(context)) + unit.getRadius();
}

bool MakeTargetCandidates::is_in_my_range(const model::Minion& unit) const {
    if (unit.getFaction() == model::FACTION_NEUTRAL) {
        const auto distance = get_position(unit).distance(get_position(context.self()));
        return distance <= std::min(max_distance, get_max_distance_for_neutral_minion_candidate(context)) + unit.getRadius();
    } else {
        return is_in_my_range(static_cast<const model::CircularUnit&>(unit));
    }
}

struct SetResult {
    const Context& context;
    Target& result;

    template <class Iterator>
    void operator ()(Iterator candidate) {
        impl(*candidate->first);
    }

    template <class Unit>
    void impl(const Unit& candidate) {
        const GetAttackRange get_attack_range {context};
        const auto optimal_position = get_optimal_position(context, &candidate, 2 * context.self().getVisionRange(),
            OPTIMAL_POSITION_INITIAL_POINTS_COUNT, OPTIMAL_POSITION_MINIMIZE_MAX_FUNCTION_CALLS);
        auto min_distance = std::min(get_position(context.self()).distance(get_position(candidate)),
                                     optimal_position.distance(get_position(candidate)));
        if (min_distance <= get_attack_range(context.self(), min_distance) + candidate.getRadius()) {
            result = get_id(candidate);
        }
    }

    void impl(const model::Tree& candidate) {
        const GetAttackRange get_attack_range {context};
        auto min_distance = get_position(context.self()).distance(get_position(candidate));
        if (min_distance <= get_attack_range(context.self(), min_distance) + candidate.getRadius()) {
            result = get_id(candidate);
        }
    }
};

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
        const LessByScore less_by_score {ends};

        Target result;
        SetResult set_result {context, result};

        for (auto iterators = begins; iterators != ends;) {
            context.check_timeout(__PRETTY_FUNCTION__, __FILE__, __LINE__);

            const auto max = max_element(iterators, less_by_score);

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

double get_max_distance_for_tree_candidate(const Context& context) {
    return 0.9 * context.game().getStaffRange();
}

double get_max_distance_for_neutral_minion_candidate(const Context& context) {
    return 0.8 * context.game().getStaffRange();
}

double get_max_distance_for_unit_candidate(const Context& context) {
    return 1.3 * context.self().getVisionRange();
}

bool has_candidates(const Context& context, double max_distance) {
    const MakeTargetCandidates make_target_candidates {context, max_distance};

    const auto has = [&] (const auto& units) {
        return units.end() != std::find_if(units.begin(), units.end(),
            [&] (const auto& v) { return make_target_candidates.is_candidate(v); });
    };

    return has(get_units<model::Bonus>(context.cache()))
               || has(get_units<model::Building>(context.cache()))
               || has(get_units<model::Minion>(context.cache()))
               || has(get_units<model::Tree>(context.cache()))
               || has(get_units<model::Wizard>(context.cache()));
}

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
