#include "optimal_position.hpp"

#ifdef STRATEGY_DEBUG

#include "debug/output.hpp"

#include <iostream>

#endif

namespace strategy {

bool is_friend(const model::Unit& unit, model::Faction my_faction, UnitId) {
    return unit.getFaction() == my_faction;
}

bool is_friend(const model::Wizard& unit, model::Faction my_faction, UnitId my_id) {
    return unit.getFaction() == my_faction && unit.getId() != my_id;
}

double get_distance_penalty(double value, double safe) {
    return std::min(1.0, std::max(0.0, (safe - value) / safe));
}

Point get_optimal_position(const Context& context, const Target& target, double max_distance) {
    if (const auto unit = target.unit<model::Bonus>(context.cache())) {
        return get_optimal_position(context, unit, max_distance);
    } else if (const auto unit = target.unit<model::Building>(context.cache())) {
        return get_optimal_position(context, unit, max_distance);
    } else if (const auto unit = target.unit<model::Minion>(context.cache())) {
        return get_optimal_position(context, unit, max_distance);
    } else if (const auto unit = target.unit<model::Wizard>(context.cache())) {
        return get_optimal_position(context, unit, max_distance);
    } else if (const auto unit = target.unit<model::Tree>(context.cache())) {
        return get_optimal_position(context, unit, max_distance);
    }
    std::ostringstream error;
    error << "Target is not set in " << __PRETTY_FUNCTION__ << " at " << __FILE__ << ":" << __LINE__;
    throw std::logic_error(error.str());
}

bool is_me(const model::Wizard& unit) {
    return unit.isMe();
}

bool is_me(const model::Unit&) {
    return false;
}

double GetAttackRange::operator ()(const model::Unit&) const {
    return 0.0;
}

double GetAttackRange::operator ()(const model::Building& unit) const {
    return context.self().getRadius() + unit.getAttackRange();
}

double GetAttackRange::operator ()(const model::Minion& unit) const {
    switch (unit.getType()) {
        case model::_MINION_UNKNOWN_:
            break;
        case model::MINION_ORC_WOODCUTTER:
            return context.self().getRadius() + context.game().getOrcWoodcutterAttackRange();
        case model::MINION_FETISH_BLOWDART:
            return context.self().getRadius() + context.game().getFetishBlowdartAttackRange()
                    + context.game().getDartRadius();
        case model::_MINION_COUNT_:
            break;
    }
    std::ostringstream error;
    error << "Invalid minion type: " << int(unit.getType())
          << " in " << __PRETTY_FUNCTION__ << " at " << __FILE__ << ":" << __LINE__;
    throw std::logic_error(error.str());
}

double GetAttackRange::operator ()(const model::Wizard& unit) const {
    return context.self().getRadius() + unit.getCastRange() + context.game().getMagicMissileRadius();
}

double GetUnitIntersectionPenalty::operator ()(const model::CircularUnit& unit, const Point& position) const {
    return get_distance_penalty(position.distance(get_position(unit)),
                                1.1 * context.self().getRadius() + unit.getRadius());
}

double GetUnitIntersectionPenalty::operator ()(const model::Tree& unit, const Point& position) const {
    const auto distance = position.distance(get_position(unit));
    const auto safe_distance = 1.1 * context.self().getRadius() + 4 * unit.getRadius();
    if (distance < safe_distance * 0.5) {
        return get_distance_penalty(distance, safe_distance);
    } else {
        return 0.5 * get_distance_penalty(distance, 2 * safe_distance);
    }
}

double GetUnitDangerPenalty::operator ()(const model::Minion& unit, const Point& position, double damage_factor, double sum_enemy_damage) const {
    if (!is_enemy(unit, context.self().getFaction())) {
        return 0;
    }
    if (friend_units.empty()) {
        return get_common(unit, position, damage_factor, sum_enemy_damage);
    }
    const auto unit_position = get_position(unit);
    const auto nearest_friend = std::min_element(friend_units.begin(), friend_units.end(),
        [&] (auto lhs, auto rhs) {
            return unit_position.distance(get_position(*lhs)) < unit_position.distance(get_position(*rhs));
        });
    if (unit_position.distance(get_position(**nearest_friend)) < unit_position.distance(position)) {
        return 0;
    }
    return get_common(unit, position, damage_factor, sum_enemy_damage);
}

double GetUnitAttackAbility::operator ()(const model::Building& unit) const {
    return 1.0 - double(unit.getRemainingActionCooldownTicks()) / double(unit.getCooldownTicks());
}

double GetUnitAttackAbility::operator ()(const model::Minion& unit) const {
    return 1.0 - double(unit.getRemainingActionCooldownTicks()) / double(unit.getCooldownTicks());
}

double GetUnitAttackAbility::operator ()(const model::Wizard& unit) const {
    return 1.0 - double(unit.getRemainingActionCooldownTicks()) / double(context.game().getWizardActionCooldownTicks());
}

}
