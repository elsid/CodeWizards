#ifndef STRATEGY_OPTIMAL_POSITION_HPP
#define STRATEGY_OPTIMAL_POSITION_HPP

#include "context.hpp"
#include "minimize.hpp"
#include "circle.hpp"
#include "optimal_target.hpp"
#include "optimal_movement.hpp"

#include <stdexcept>
#include <algorithm>
#include <numeric>
#include <type_traits>
#include <iostream>

namespace strategy {

double get_distance_penalty(double value, double safe);

Point get_optimal_position(const Context& context, const Target& target, double max_distance,
                           std::size_t initial_points_count, int max_function_calls);

struct GetVisionRange {
    const Context& context;

    double operator ()(const model::Unit&) const;
    double operator ()(const model::Building& unit) const;
    double operator ()(const model::Minion& unit) const;
    double operator ()(const model::Wizard& unit) const;
};

struct GetUnitIntersectionPenalty {
    const Context& context;

    double operator ()(const model::CircularUnit& unit, const Point& position) const;
    double operator ()(const model::Minion& unit, const Point& position) const;
    double operator ()(const model::Tree& unit, const Point& position) const;

    double base(const model::CircularUnit& unit, const Point& position) const;
    double increased(const model::CircularUnit& unit, const Point& position) const;
};

struct GetRangedDamage {
    const Context& context;

    template <class Unit>
    double operator ()(const Unit& unit, const Point& position) const {
        const GetAttackRange get_attack_range {context};
        const GetMaxDamage get_damage {context};
        const auto current_distance = position.distance(get_position(unit));
        const auto future_distance = position.distance(get_position(unit) + get_speed(unit));
        const auto distance = std::min(current_distance, future_distance);
        const auto attack_range = get_attack_range(unit, distance) + context.self().getRadius();
        const auto factor = attack_range >= current_distance || attack_range >= future_distance
                ? 1.0
                : get_distance_penalty(std::min(current_distance, future_distance) - attack_range,
                                       2 * context.self().getRadius());
        return factor * get_damage(unit, distance);
    }
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

struct GetCurrentDamage {
    const Context& context;

    template <class Unit>
    double operator ()(const Unit& unit, const Point& position) const {
        const GetRangedDamage get_ranged_damage {context};
        const GetUnitAttackAbility get_attack_ability {context};
        return get_ranged_damage(unit, position) * get_attack_ability(unit) * get_turn_factor(unit, position);
    }

    template <class Unit>
    double get_turn_factor(const Unit&, const Point&) const {
        return 1.0;
    }

    double get_turn_factor(const model::Wizard& unit, const Point& position) const {
        return get_rotating_unit_turn_factor(unit, position);
    }

    template <class Unit>
    double get_rotating_unit_turn_factor(const Unit& unit, const Point& position) const {
        const Bounds my_bounds(context);
        const auto unit_bounds = make_unit_bounds(context, unit);
        const auto my_position = get_position(context.self());
        const auto current_angle = unit.getAngle();
        const auto future_direction = position - get_position(unit);
        const auto future_angle = normalize_angle(future_direction.absolute_rotation() - unit.getAngle());
        const auto turn = std::max(0.0, std::abs(future_angle - current_angle) - context.game().getWizardMaxTurnAngle());
        const auto turn_ticks = turn / unit_bounds.max_turn(0);
        const auto distance = position.distance(my_position);
        const auto move_ticks = distance / my_bounds.max_strafe_speed(std::ceil(turn_ticks));
        const auto max_cast_angle = context.game().getStaffSector() / 2;
        return turn_ticks > move_ticks ? 1.0 - std::max(0.0, future_angle - max_cast_angle) / (M_PI - max_cast_angle) : 1.0;
    }
};

struct GetUnitDangerPenalty {
    const Context& context;
    const std::vector<const model::CircularUnit*>& friend_units;

    template <class T>
    double operator ()(const T& unit, const Point& position, double sum_damage_to_me) const {
        return is_enemy(unit, context.self().getFaction()) ? get_common(unit, position, sum_damage_to_me) : 0;
    }

    double operator ()(const model::Minion& unit, const Point& position, double sum_damage_to_me) const;

    template <class T>
    double get_common(const T& unit, const Point& position, double sum_damage_to_me) const {
        const Bounds my_bounds(context);
        const auto time_to_position = get_position(context.self()).distance(position) / my_bounds.max_speed(0);
        const auto& cached_unit = get_units<T>(context.cache()).at(unit.getId());
        const auto unit_future_position = get_position(unit) + cached_unit.mean_speed() * time_to_position;
        const GetAttackRange get_attack_range {context};
        const auto current_distance = position.distance(get_position(unit));
        const auto future_distance = position.distance(unit_future_position);
        const auto max_distance = std::max(current_distance, future_distance);
        const auto min_distance = std::min(current_distance, future_distance);
        const auto distance_factor = 2 * sum_damage_to_me / context.self().getLife();
        const auto safe_distance = get_attack_range(unit, max_distance)
                + (1 + distance_factor) * context.self().getRadius();
        return get_distance_penalty(min_distance, safe_distance);
    }
};

template <class T>
class GetPositionPenalty {
public:
    using Target = T;

    GetPositionPenalty(const Context& context, const Target* target, double max_distance)
            : context(context),
              target(target),
              max_distance(max_distance) {
        const IsInMyRange is_in_my_range {context, max_distance};

        const auto initial_filter = [&] (const auto& units) {
            return filter_units(units, [&] (const auto& unit) { return !is_me(unit) && is_in_my_range(unit); });
        };

        buildings = initial_filter(get_units<model::Building>(context.cache()));
        minions = initial_filter(get_units<model::Minion>(context.cache()));
        wizards = initial_filter(get_units<model::Wizard>(context.cache()));
        trees = initial_filter(get_units<model::Tree>(context.cache()));
        bonuses = initial_filter(get_units<model::Bonus>(context.cache()));

        const auto is_enemy = [&] (const auto& unit) {
            return strategy::is_enemy(unit, context.self().getFaction());
        };

        enemy_wizards = filter_units(wizards, is_enemy);
        enemy_minions = filter_units(minions, is_enemy);
        enemy_buildings = filter_units(buildings, is_enemy);

        friend_wizards = filter_friends(wizards, context.self().getFaction(), context.self().getId());
        const auto friend_minions = filter_friends(minions, context.self().getFaction(), context.self().getId());
        friend_buildings = filter_friends(buildings, context.self().getFaction(), context.self().getId());

        friend_units.reserve(friend_wizards.size() + friend_minions.size() + friend_buildings.size());
        std::copy(friend_wizards.begin(), friend_wizards.end(), std::back_inserter(friend_units));
        std::copy(friend_minions.begin(), friend_minions.end(), std::back_inserter(friend_units));
        std::copy(friend_buildings.begin(), friend_buildings.end(), std::back_inserter(friend_units));

        const GetDefenceFactor get_defence_factor {context};

        my_defence_factor = get_defence_factor(context.self());
        max_borders_penalty = double(bonuses.size() + buildings.size() + minions.size() + trees.size() + wizards.size()
                                     + get_units<model::Projectile>(context.cache()).size() * PROJECTILE_PENALTY_WEIGHT);
    }

    double operator ()(const Point& position) const {
        context.check_timeout(__PRETTY_FUNCTION__, __FILE__, __LINE__);

        if (is_out_of_borders(position)) {
            return get_borders_penalty(position);
        }

        const auto penalties = {
            get_units_danger_penalty(position) * UNITS_DANGER_PENALTY_WEIGHT,
            get_units_collision_penalty(position),
            get_bonuses_penalty(position),
            get_projectiles_penalty(position) * PROJECTILE_PENALTY_WEIGHT,
            get_friendly_fire_penalty(position),
            get_target_penalty(position),
            get_borders_penalty(position),
        };

        const auto max_penalty = *std::max_element(penalties.begin(), penalties.end());
        const auto score = get_elimination_score(position) * ELIMINATION_SCORE_WEIGHT;

        return max_penalty - score;
    }

    bool is_out_of_borders(const Point& position) const {
        return context.self().getRadius() >= position.x()
            || position.x() >= context.game().getMapSize() - context.self().getRadius()
            || context.self().getRadius() >= position.y()
            || position.y() >= context.game().getMapSize() - context.self().getRadius();
    }

    double get_borders_penalty(const Point& position) const {
        const std::array<double, 4> factors = {{
            get_borders_factor(position.x()),
            get_borders_factor(context.game().getMapSize() - position.x()),
            get_borders_factor(position.y()),
            get_borders_factor(context.game().getMapSize() - position.y()),
        }};
        const auto max_factor = std::max_element(factors.begin(), factors.end());
        return max_borders_penalty * *max_factor;
    }

    double get_projectiles_penalty(const Point& position) const {
        const auto& projectiles = get_units<model::Projectile>(context.cache());
        return std::accumulate(projectiles.begin(), projectiles.end(), 0.0,
            [&] (auto sum, auto v) { return sum + this->get_projectile_penalty(v.second, position); });
    }

    double get_elimination_score(const Point& position) const {
        const auto get_sum_elimination_score = [&] (const auto& units) {
            return std::accumulate(units.begin(), units.end(), 0.0,
                [&] (auto sum, const auto& v) { return sum + this->get_elimination_score(v.second, position); });
        };

        return get_sum_elimination_score(get_units<model::Building>(context.cache()))
                + get_sum_elimination_score(get_units<model::Minion>(context.cache()))
                + get_sum_elimination_score(get_units<model::Wizard>(context.cache()));
    }

    double get_sum_damage_to_me(const Point& position) const {
        const auto add_damage = [&] (auto sum, auto v) {
            return sum + this->get_unit_current_damage(*v, position);
        };

        const double enemy_wizards_damage = std::accumulate(enemy_wizards.begin(), enemy_wizards.end(), 0.0, add_damage);
        const double enemy_minions_damage = std::accumulate(enemy_minions.begin(), enemy_minions.end(), 0.0, add_damage);
        const double enemy_buildings_damage = std::accumulate(enemy_buildings.begin(), enemy_buildings.end(), 0.0, add_damage);

        return enemy_wizards_damage
                + enemy_minions_damage
                + enemy_buildings_damage
                + is_with_status(context.self(), model::STATUS_BURNING)
                    * double(context.game().getBurningSummaryDamage()) / double(context.game().getBurningDurationTicks());
    }

    double get_units_danger_penalty(const Point& position) const {
        const auto sum_damage_to_me = get_sum_damage_to_me(position);
        return get_units_danger_penalty(enemy_buildings, position, sum_damage_to_me)
                + get_units_danger_penalty(enemy_minions, position, sum_damage_to_me)
                + get_units_danger_penalty(enemy_wizards, position, sum_damage_to_me);
    }

    double get_units_collision_penalty(const Point& position) const {
        return get_units_collision_penalty(buildings, position)
                + get_units_collision_penalty(minions, position)
                + get_units_collision_penalty(trees, position)
                + get_units_collision_penalty(wizards, position);
    }

    double get_bonuses_penalty(const Point& position) const {
        return std::accumulate(bonuses.begin(), bonuses.end(), 0.0,
            [&] (auto sum, auto v) { return sum + this->get_bonus_penalty(*v, position); });
    }

    double get_friendly_fire_penalty(const Point& position) const {
        return get_friendly_fire_penalty(friend_buildings, position)
                + get_friendly_fire_penalty(friend_wizards, position);
    }

    double get_target_penalty(const Point& position) const {
        return target ? get_target_penalty(*target, position) : 0;
    }

private:
    const Context& context;
    const Target* const target;
    const double max_distance;
    std::vector<const model::Bonus*> bonuses;
    std::vector<const model::Building*> buildings;
    std::vector<const model::Minion*> minions;
    std::vector<const model::Tree*> trees;
    std::vector<const model::Wizard*> wizards;
    std::vector<const model::Wizard*> enemy_wizards;
    std::vector<const model::Minion*> enemy_minions;
    std::vector<const model::Building*> enemy_buildings;
    std::vector<const model::CircularUnit*> friend_units;
    std::vector<const model::Wizard*> friend_wizards;
    std::vector<const model::Building*> friend_buildings;
    double max_borders_penalty = std::numeric_limits<double>::max();
    double my_defence_factor = 1;

    double get_bonus_penalty(const model::Bonus& unit, const Point& position) const {
        const auto distance = position.distance(get_position(unit));
        return 1 - get_distance_penalty(distance, 2 * context.self().getVisionRange());
    }

    double get_projectile_penalty(const CachedUnit<model::Projectile>& cached_unit, const Point& position) const {
        const auto& unit = cached_unit.value();
        const auto trajectory = get_projectile_trajectory(cached_unit);
        const auto lethal_area = get_projectile_lethal_area(unit.getType());
        const auto nearest = trajectory.nearest(position);
        double distance_to;
        if (trajectory.has_point(nearest)) {
            distance_to = position.distance(nearest);
        } else {
            distance_to = std::min(position.distance(trajectory.begin()), position.distance(trajectory.end()));
        }
        if (distance_to <= lethal_area + unit.getRadius()) {
            return 0.9 + 0.1 * line_factor(distance_to, lethal_area + unit.getRadius(), 0);
        } else {
            return 0.9 * line_factor(distance_to, 2 * (lethal_area + unit.getRadius()), lethal_area + unit.getRadius());
        }
    }

    Line get_projectile_trajectory(const CachedUnit<model::Projectile>& cached_unit) const {
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

    double get_projectile_lethal_area(model::ProjectileType type) const {
        switch (type) {
            case model::PROJECTILE_MAGIC_MISSILE:
                return context.game().getMagicMissileRadius();
            case model::PROJECTILE_FROST_BOLT:
                return context.game().getFrostBoltRadius();
            case model::PROJECTILE_FIREBALL:
                return context.game().getFireballExplosionMinDamageRange();
            case model::PROJECTILE_DART:
                return context.game().getDartRadius();
            default:
                break;
        }
        std::ostringstream error;
        error << "Invalid model::ProjectileType value: " << int(type)
              << " in " << __PRETTY_FUNCTION__ << " at " << __FILE__ << ":" << __LINE__;
        throw std::logic_error(error.str());
    }

    double get_friendly_fire_penalty(const model::CircularUnit& unit, const Point& position) const {
        if (!target) {
            return 0.0;
        }

        const auto target_position = get_position(*target);
        const auto unit_position = get_position(unit);
        const auto has_intersection = Circle(unit_position, unit.getRadius())
                .has_intersection(Circle(position, context.self().getRadius()), target_position);

        if (!has_intersection) {
            return 0.0;
        }

        const auto cast_radius = std::max(context.game().getMagicMissileRadius(),
            std::max(context.game().getFrostBoltRadius(), context.game().getFireballRadius()));
        const auto target_to_unit = unit_position - target_position;
        const auto tangent_cos = (unit.getRadius(), cast_radius) / target_position.distance(unit_position);
        const auto tangent_angle = std::acos(std::min(1.0, std::max(-1.0, tangent_cos)));
        const auto tangent1_direction = target_to_unit.rotated(tangent_angle);
        const auto tangent2_direction = target_to_unit.rotated(-tangent_angle);
        const auto tangent1 = target_position + tangent1_direction;
        const auto tangent2 = target_position + tangent2_direction;
        const auto tangent1_distance = Line(target_position, tangent1).distance(position);
        const auto tangent2_distance = Line(target_position, tangent2).distance(position);
        const auto max_distance = (tangent1_distance + tangent2_distance) * 0.5;
        const auto distance_to_tangent = std::min(tangent1_distance, tangent2_distance);
        return distance_to_tangent / max_distance;
    }

    template <class Unit>
    double get_elimination_score(const CachedUnit<Unit>& unit, const Point& position) const {
        return get_base_elimination_score(unit, position);
    }

    double get_elimination_score(const CachedUnit<model::Building>& unit, const Point& position) const {
        return unit.value().getType() == model::BUILDING_FACTION_BASE ? 0 : get_base_elimination_score(unit, position);
    }

    template <class Unit>
    double get_base_elimination_score(const CachedUnit<Unit>& unit, const Point& position) const {
        const auto mean_life_change_speed = unit.mean_life_change_speed();

        if (!is_enemy(unit.value(), context.self().getFaction()) || mean_life_change_speed >= 0) {
            return 0;
        }

        const auto distance = get_position(unit.value()).distance(position);
        const auto factor = line_factor(-mean_life_change_speed * 30, 0, unit.value().getLife());

        if (distance <= context.game().getScoreGainRange() - context.self().getRadius()) {
            return factor * (1 + 0.1 * line_factor(distance, context.game().getScoreGainRange()- context.self().getRadius(), 0));
        } else {
            return factor * line_factor(distance, context.game().getScoreGainRange(), context.game().getScoreGainRange() - context.self().getRadius());
        }
    }

    double get_borders_factor(double distance) const {
        return std::max(0.0, (2 * context.self().getRadius() - distance) / (2 * context.self().getRadius()));
    }

    template <class Unit>
    double get_unit_current_damage(const Unit& unit, const Point& position) const {
        const GetCurrentDamage get_current_damage {context};
        return my_defence_factor * get_current_damage(unit, position);
    }

    template <class Unit>
    double get_units_danger_penalty(const std::vector<const Unit*>& units, const Point& position, double sum_damage_to_me) const {
        const GetUnitDangerPenalty get_unit_danger_penalty {context, friend_units};
        return std::accumulate(units.begin(), units.end(), 0.0,
            [&] (auto sum, auto v) { return sum + get_unit_danger_penalty(*v, position, sum_damage_to_me); });
    }

    template <class Unit>
    double get_units_collision_penalty(const std::vector<const Unit*>& units, const Point& position) const {
        const GetUnitIntersectionPenalty get_unit_collision_penalty {context};
        return std::accumulate(units.begin(), units.end(), 0.0,
           [&] (auto sum, auto v) { return sum + get_unit_collision_penalty(*v, position); });
    }

    template <class Unit>
    double get_friendly_fire_penalty(const std::vector<const Unit*>& units, const Point& position) const {
        return std::accumulate(units.begin(), units.end(), 0.0,
            [&] (auto sum, auto v) { return sum + this->get_friendly_fire_penalty(*v, position); });
    }

    template <class Unit>
    double get_target_penalty(const Unit& unit, const Point& position) const {
        const GetAttackRange get_attack_range {context};
        const GetMaxDamage get_max_damage {context};
        const auto current_distance = position.distance(get_position(unit));
        const auto future_distance = position.distance(get_position(unit) + get_speed(unit));
        const auto distance = std::max(current_distance, future_distance);
        const auto range = get_attack_range(context.self(), distance);
        const auto ticks_to_action = get_max_damage.next_attack_action(context.self(), distance).second;
        const auto ticks_factor = 1 - line_factor(ticks_to_action, 0, context.game().getWizardActionCooldownTicks());
        const auto damage = get_unit_current_damage(unit, position);
        if (distance <= range) {
            return 0.01 * line_factor(distance, 0, range) * ticks_factor;
        } else {
            return (0.01 + line_factor(distance, range, 2 * range)
                    * line_factor(context.self().getLife(), damage, context.self().getMaxLife())) * ticks_factor;
        }
    }

    double get_target_penalty(const model::Bonus& unit, const Point& position) const {
        return get_bonus_penalty(unit, position);
    }
};

template <class T>
Point get_optimal_position(const Context& context, const T* target, double max_distance,
                           std::size_t initial_points_count, int max_function_calls) {
    const GetPositionPenalty<T> get_position_penalty(context, target, max_distance);
    std::vector<std::pair<double, Point>> points;
    points.reserve(initial_points_count);
    points.emplace_back(minimize(get_position_penalty, get_position(context.self()), max_function_calls));
    std::size_t step = 0;
    std::generate_n(std::back_inserter(points), initial_points_count - 1,
        [&] {
            context.check_timeout(__PRETTY_FUNCTION__, __FILE__, __LINE__);
            const auto angle = normalize_angle(2.0 * M_PI * double(step++) / double(initial_points_count));
            const auto initial = get_position(context.self()) + Point(1, 0).rotated(angle) * 0.25 * context.self().getVisionRange();
            return minimize(get_position_penalty, initial, max_function_calls);
        });
    const auto result = std::min_element(points.begin(), points.end(),
        [] (const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; })->second;
    return result;
}

template <>
inline Point get_optimal_position(const Context& /*context*/, const model::Bonus* target, double /*max_distance*/,
                                  std::size_t /*initial_points_count*/, int /*max_function_calls*/) {
    return get_position(*target);
}

}

#endif
