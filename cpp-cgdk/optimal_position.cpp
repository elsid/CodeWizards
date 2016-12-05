#include "optimal_position.hpp"

#ifdef STRATEGY_DEBUG

#include "debug/output.hpp"

#include <iostream>

#endif

namespace strategy {

double get_distance_penalty(double value, double safe) {
    return std::min(1.0, std::max(0.0, (safe - value) / safe));
}

Point get_optimal_position(const Context& context, const Target& target, double max_distance,
                           std::size_t initial_points_count, int max_function_calls) {
    if (const auto unit = target.unit<model::Bonus>(context.cache())) {
        return get_optimal_position(context, unit, max_distance, initial_points_count, max_function_calls);
    } else if (const auto unit = target.unit<model::Building>(context.cache())) {
        return get_optimal_position(context, unit, max_distance, initial_points_count, max_function_calls);
    } else if (const auto unit = target.unit<model::Minion>(context.cache())) {
        return get_optimal_position(context, unit, max_distance, initial_points_count, max_function_calls);
    } else if (const auto unit = target.unit<model::Wizard>(context.cache())) {
        return get_optimal_position(context, unit, max_distance, initial_points_count, max_function_calls);
    } else if (const auto unit = target.unit<model::Tree>(context.cache())) {
        return get_optimal_position(context, unit, max_distance, initial_points_count, max_function_calls);
    }
    std::ostringstream error;
    error << "Target is not set in " << __PRETTY_FUNCTION__ << " at " << __FILE__ << ":" << __LINE__;
    throw std::logic_error(error.str());
}

double GetVisionRange::operator ()(const model::Unit&) const {
    return 0.0;
}

double GetVisionRange::operator ()(const model::Building& unit) const {
    return unit.getVisionRange();
}

double GetVisionRange::operator ()(const model::Minion& unit) const {
    return unit.getVisionRange();
}

double GetVisionRange::operator ()(const model::Wizard& unit) const {
    return unit.getVisionRange();
}

double GetUnitIntersectionPenalty::operator ()(const model::CircularUnit& unit, const Point& position) const {
    return base(unit, position);
}

double GetUnitIntersectionPenalty::operator ()(const model::Minion& unit, const Point& position) const {
    if (unit.getFaction() == model::FACTION_NEUTRAL) {
        return increased(unit, position);
    } else {
        return base(unit, position);
    }
}

double GetUnitIntersectionPenalty::operator ()(const model::Tree& unit, const Point& position) const {
    return increased(unit, position);
}

double GetUnitIntersectionPenalty::increased(const model::CircularUnit& unit, const Point& position) const {
    const auto distance = position.distance(get_position(unit));
    const auto safe_distance = 1.1 * context.self().getRadius() + 2 * unit.getRadius();
    if (distance < safe_distance * 0.5) {
        return get_distance_penalty(distance, safe_distance);
    } else {
        return 0.5 * get_distance_penalty(distance, 2.0 * safe_distance);
    }
}

double GetUnitIntersectionPenalty::base(const model::CircularUnit& unit, const Point& position) const {
    return get_distance_penalty(position.distance(get_position(unit)),
                                1.1 * context.self().getRadius() + unit.getRadius());
}

double GetUnitDangerPenalty::operator ()(const model::Minion& unit, const Point& position, double sum_enemy_damage) const {
    if (!is_enemy(unit, context.self().getFaction())) {
        return 0;
    }
    if (friend_units.empty()) {
        return get_common(unit, position, sum_enemy_damage);
    }
    const auto unit_position = get_position(unit);
    const auto nearest_friend = std::min_element(friend_units.begin(), friend_units.end(),
        [&] (auto lhs, auto rhs) {
            return unit_position.distance(get_position(*lhs)) < unit_position.distance(get_position(*rhs));
        });
    if (unit_position.distance(get_position(**nearest_friend)) < unit_position.distance(position)) {
        return 0;
    }
    return get_common(unit, position, sum_enemy_damage);
}

double GetUnitAttackAbility::operator ()(const model::Building& unit) const {
    const auto last_seen = get_units<model::Building>(context.cache()).at(unit.getId()).last_seen();
    const auto remaining = std::max(unit.getRemainingActionCooldownTicks() - (context.world().getTickIndex() - last_seen), 0);
    return 1.0 - 0.5 * double(remaining) / double(unit.getCooldownTicks());
}

double GetUnitAttackAbility::operator ()(const model::Minion& unit) const {
    return 1.0 - 0.5 * double(unit.getRemainingActionCooldownTicks()) / double(unit.getCooldownTicks());
}

double GetUnitAttackAbility::operator ()(const model::Wizard& unit) const {
    return 1.0 - 0.5 * double(unit.getRemainingActionCooldownTicks()) / double(context.game().getWizardActionCooldownTicks());
}

}
