#include "optimal_position.hpp"

#ifdef ELSID_STRATEGY_DEBUG

#include "debug/output.hpp"

#include <iostream>

#endif

namespace strategy {

double get_distance_penalty(double value, double safe) {
    return std::min(1.0, std::max(0.0, (safe - value) / safe));
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
    const auto to_position = position - get_position(context.self());
    const auto angle = std::abs(normalize_angle(to_position.absolute_rotation() - context.self().getAngle()));
    if (angle < M_PI_4) {
        return base(unit, position);
    } else {
        return increased(unit, position);
    }
}

double GetUnitIntersectionPenalty::increased(const model::CircularUnit& unit, const Point& position) const {
    const auto distance = position.distance(get_position(unit));
    const auto safe_distance = context.game().getStaffRange() + unit.getRadius();
    if (distance < safe_distance * 0.5) {
        return get_distance_penalty(distance, safe_distance);
    } else {
        return 0.5 * line_factor(distance, 2.0 * safe_distance, 0);
    }
}

double GetUnitIntersectionPenalty::base(const model::CircularUnit& unit, const Point& position) const {
    return line_factor(position.distance(get_position(unit)), context.game().getStaffRange() + unit.getRadius(), 0);
}

double GetUnitDangerPenalty::operator ()(const model::Minion& unit, const Point& position, double sum_enemy_damage) const {
    const Bounds my_bounds(context);
    const auto time_to_position = get_position(context.self()).distance(position) / my_bounds.max_speed(0);
    const auto& cached_unit = get_units<model::Minion>(context.cache()).at(unit.getId());
    const auto unit_position = get_position(unit) + cached_unit.mean_speed() * time_to_position;
    const auto distance_to_me = std::max(unit_position.distance(position), unit.getRadius() + context.self().getRadius());
    if (friend_units.empty()) {
        return get_base(unit, position, sum_enemy_damage);
    }
    const auto nearest_friend = std::min_element(friend_units.begin(), friend_units.end(),
        [&] (auto lhs, auto rhs) {
            return unit_position.distance(get_position(*lhs)) < unit_position.distance(get_position(*rhs));
        });
    const auto distance_to_nearest = std::max(unit_position.distance(get_position(**nearest_friend)),
                                              unit.getRadius() + (*nearest_friend)->getRadius());
    if (distance_to_me - distance_to_nearest > context.self().getRadius()) {
        return line_factor(distance_to_me, distance_to_nearest, 0);
    }
    return get_base(unit, position, sum_enemy_damage);
}

double GetUnitAttackAbility::operator ()(const model::Building& unit) const {
    const auto last_seen = get_units<model::Building>(context.cache()).at(unit.getId()).last_seen();
    const auto remaining = std::max(unit.getRemainingActionCooldownTicks() - (context.world().getTickIndex() - last_seen), 0);
    return 1.0 - double(remaining) / double(unit.getCooldownTicks());
}

double GetUnitAttackAbility::operator ()(const model::Minion& unit) const {
    const auto frozen = find_status(unit.getStatuses(), model::STATUS_FROZEN);
    const auto remaining = std::max(unit.getRemainingActionCooldownTicks(),
                                    frozen == unit.getStatuses().end() ? 0 : frozen->getRemainingDurationTicks());
    return 1.0 - double(remaining) / double(std::max(unit.getCooldownTicks(), remaining));
}

double GetUnitAttackAbility::operator ()(const model::Wizard& unit) const {
    const auto frozen = find_status(unit.getStatuses(), model::STATUS_FROZEN);
    const auto remaining = std::max(unit.getRemainingActionCooldownTicks(),
                                    frozen == unit.getStatuses().end() ? 0 : frozen->getRemainingDurationTicks());
    return 1.0 - double(remaining) / double(std::max(context.game().getWizardActionCooldownTicks(), remaining));
}

}
