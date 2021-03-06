#include "optimal_position.hpp"

#ifdef ELSID_STRATEGY_DEBUG

#include "debug/output.hpp"

#include <iostream>

#endif

namespace strategy {

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
    const auto safe_distance = get_safe_distance(unit);
    if (distance < safe_distance * 0.5) {
        return line_factor(distance, safe_distance, 0);
    } else {
        return 0.5 * line_factor(distance, 2.0 * safe_distance, 0);
    }
}

double GetUnitIntersectionPenalty::base(const model::CircularUnit& unit, const Point& position) const {
    return line_factor(position.distance(get_position(unit)), get_safe_distance(unit), 0);
}

double GetUnitIntersectionPenalty::get_safe_distance(const model::CircularUnit& unit) const {
    return context.game().getStaffRange() + unit.getRadius();
}

double GetUnitDangerPenalty::operator ()(const model::Minion& unit, const Point& position, double sum_enemy_damage) const {
    const auto my_bounds = make_unit_bounds(context.self(), context.game());
    const auto time_to_position = get_position(context.self()).distance(position) / my_bounds.max_speed(0);
    const auto& cached_unit = get_units<model::Minion>(context.cache()).at(unit.getId());
    const auto unit_position = get_position(unit) + cached_unit.mean_speed() * time_to_position;
    const auto distance_to_me = unit_position.distance(position);

    if (distance_to_me < context.game().getStaffRange() + unit.getRadius() + context.self().getRadius()) {
        return get_base(unit, position, sum_enemy_damage);
    }

    if (friend_units.empty()) {
        return get_base(unit, position, sum_enemy_damage);
    }
    const auto nearest_friend = std::min_element(friend_units.begin(), friend_units.end(),
        [&] (auto lhs, auto rhs) {
            return unit_position.distance(get_position(*lhs)) < unit_position.distance(get_position(*rhs));
        });
    const auto distance_to_nearest = std::max({
        unit_position.distance(get_position(**nearest_friend)),
        unit.getRadius() + (*nearest_friend)->getRadius(),
        unit.getRadius() + context.game().getStaffRange(),
    });
    if (distance_to_me - distance_to_nearest > context.self().getRadius()) {
        return line_factor(distance_to_me, distance_to_nearest, 0);
    }
    return get_base(unit, position, sum_enemy_damage);
}

double GetUnitDangerPenalty::operator ()(const model::Building& unit, const Point& position, double sum_damage_to_me) const {
    const auto base = get_base(unit, position, sum_damage_to_me);
    if (unit.getType() == model::BUILDING_FACTION_BASE) {
        const Point corner(context.world().getWidth(), 0);
        const auto distance = position.distance(corner);
        const auto from_prev_minions_spawn = context.world().getTickIndex() % MINIONS_SPAWN_PERIOD + 1;
        const auto safe_distance = context.game().getFactionBaseAttackRange() * double(from_prev_minions_spawn) / double(MINIONS_SPAWN_PERIOD + 1)
                + get_position(unit).distance(corner) + context.game().getFactionBaseRadius();
        const auto spawned_minions_danger = line_factor(distance, safe_distance, 0);
        return std::max(base, spawned_minions_danger);
    } else {
        return base;
    }
}

Line GetProjectileTrajectory::operator ()(const CachedUnit<model::Projectile>& cached_unit) const {
    const auto& unit = cached_unit.value();
    switch (unit.getType()) {
        case model::PROJECTILE_MAGIC_MISSILE:
        case model::PROJECTILE_FROST_BOLT:
        case model::PROJECTILE_FIREBALL: {
            const auto& wizards = get_units<model::Wizard>(context.history_cache());
            const auto owner = wizards.find(cached_unit.value().getOwnerUnitId());
            const auto range = owner == wizards.end() ? context.game().getWizardVisionRange()
                                                      : owner->second.value().getCastRange();
            return Line(cached_unit.first_position(), cached_unit.first_position()
                        + get_speed(cached_unit.value()).normalized() * range);
        }
        case model::PROJECTILE_DART:
            return Line(cached_unit.first_position(), cached_unit.first_position()
                        + get_speed(cached_unit.value()).normalized() * context.game().getFetishBlowdartAttackRange());
        default:
            break;
    }
    std::ostringstream error;
    error << "Invalid model::ProjectileType value: " << int(unit.getType())
          << " in " << __PRETTY_FUNCTION__ << " at " << __FILE__ << ":" << __LINE__;
    throw std::logic_error(error.str());
}

}
