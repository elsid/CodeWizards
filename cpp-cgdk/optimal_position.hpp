#pragma once

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

Point get_optimal_position(const Context& context, const Target& target, double max_distance,
                           long max_function_calls = std::numeric_limits<long>::max());

struct GetUnitIntersectionPenalty {
    const Context& context;

    double operator ()(const model::CircularUnit& unit, const Point& position) const;
    double operator ()(const model::Minion& unit, const Point& position) const;
    double operator ()(const model::Tree& unit, const Point& position) const;

    double base(const model::CircularUnit& unit, const Point& position) const;
    double increased(const model::CircularUnit& unit, const Point& position) const;
    double get_safe_distance(const model::CircularUnit& unit) const;
};

struct GetRangedDamage {
    const Context& context;

    template <class Unit>
    Damage operator ()(const Unit& unit, const Point& position) const {
        const GetAttackRange get_attack_range {context};
        const GetMaxDamage get_damage {context};
        const auto distance = position.distance(get_position(unit));
        const auto attack_range = get_attack_range(unit, distance) + context.self().getRadius();
        const auto factor = distance <= attack_range
                ? 1.0
                : bounded_line_factor(distance - attack_range, 2 * context.self().getRadius(), 0);
        const auto damage = get_damage(unit, distance);
        return factor * damage;
    }
};

struct GetCurrentDamage {
    const Context& context;

    template <class Unit>
    Damage operator ()(const Unit& unit, const Point& position) const {
        const GetRangedDamage get_ranged_damage {context};
        const GetUnitAttackAbility get_attack_ability {context};
        const auto ranged_damage = get_ranged_damage(unit, position);
        const auto attack_ability = get_attack_ability(unit);
        const auto turn_factor = get_turn_factor(unit, position);
        return ranged_damage * attack_ability * turn_factor;
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
        const auto my_bounds = make_unit_bounds(context.self(), context.game());
        const auto unit_bounds = make_unit_bounds(unit, context.game());
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
        return get_base(unit, position, sum_damage_to_me);
    }

    double operator ()(const model::Minion& unit, const Point& position, double sum_damage_to_me) const;
    double operator ()(const model::Building& unit, const Point& position, double sum_damage_to_me) const;

    template <class T>
    double get_base(const T& unit, const Point& position, double sum_damage_to_me) const {
        const GetAttackRange get_attack_range {context};
        const GetMaxDamage get_max_damage {context};
        const auto unit_position = get_position(unit);
        const auto distance = position.distance(unit_position);
        const auto permissible_damage = std::max(1.0, get_max_damage(unit, distance).sum());
        const auto distance_factor = double(context.self().getMaxLife()) / permissible_damage
                * sum_damage_to_me / context.self().getLife()
                * double(context.self().getMaxLife()) / double(context.self().getLife());
        const auto safe_distance = std::max(context.game().getStaffRange() + unit.getRadius() - 1,
                2 * context.self().getRadius() + distance_factor * get_attack_range(unit, distance));
        return line_factor(distance, safe_distance, 0);
    }
};

struct GetProjectileTrajectory {
    const Context& context;

    Line operator ()(const CachedUnit<model::Projectile>& cached_unit) const;
};

template <class Iterator, class Result, class Accumulate, class BinaryOperation>
Result cross_product(Iterator lhs_begin, Iterator lhs_end, Iterator rhs_begin, Iterator rhs_end, Result init,
        const Accumulate& accumulate, const BinaryOperation& operation) {
    for (; lhs_begin != lhs_end; ++lhs_begin) {
        for (; rhs_begin != rhs_end; ++rhs_begin) {
            init = accumulate(init, operation(*lhs_begin, *rhs_begin));
        }
    }
    return init;
}

template <class T>
class GetPositionPenalty {
public:
    using Target = T;

    static constexpr double UNITS_DANGER_PENALTY_WEIGHT = 1.2;
    static constexpr double UNITS_COLLISION_PENALTY_WEIGHT = 1.1;
    static constexpr double BONUSES_PENALTY_WEIGHT = 0.75;
    static constexpr double PROJECTILE_PENALTY_WEIGHT = 1.3;
    static constexpr double FRIENDLY_FIRE_PENALTY_WEIGHT = 0.1;
    static constexpr double TARGET_PENALTY_WEIGHT = 0.05;
    static constexpr double BORDERS_PENALTY_WEIGHT = 1.4;
    static constexpr double ELIMINATION_SCORE_WEIGHT = 0.2;
    static constexpr double FRIEND_WIZARDS_DISTANCE_PENALTY_WEIGHT = 0.1;
    static constexpr double SURROUND_PENALTY_WEIGHT = 0.75;

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
        enemy_buildings = filter_units(buildings, is_enemy);

        for (const auto& unit : get_units<model::Minion>(context.cache())) {
            if (is_enemy(unit.second.value()) || (unit.second.value().getFaction() == model::FACTION_NEUTRAL
                                                  && unit.second.is_active(context.world().getTickIndex()))) {
                enemy_minions.push_back(&unit.second.value());
            }
        }

        friend_wizards = filter_friends(wizards, context.self().getFaction());
        friend_minions = filter_friends(minions, context.self().getFaction());
        friend_buildings = filter_friends(buildings, context.self().getFaction());

        friend_units.reserve(friend_wizards.size() + friend_minions.size() + friend_buildings.size());
        std::copy(friend_wizards.begin(), friend_wizards.end(), std::back_inserter(friend_units));
        std::copy(friend_minions.begin(), friend_minions.end(), std::back_inserter(friend_units));
        std::copy(friend_buildings.begin(), friend_buildings.end(), std::back_inserter(friend_units));

        const auto make_surround_unit = [&] (auto unit) {
            if (is_friend(*unit, context.self().getFaction())) {
                return SurroundUnit {get_position(*unit), unit->getRadius() + context.self().getRadius()};
            } else {
                return this->make_enemy_surround_unit(*unit);
            }
        };

        surround_units.reserve(buildings.size() + minions.size() + wizards.size() + trees.size());
        std::transform(buildings.begin(), buildings.end(), std::back_inserter(surround_units), make_surround_unit);
        std::transform(trees.begin(), trees.end(), std::back_inserter(surround_units), make_surround_unit);
        std::transform(wizards.begin(), wizards.end(), std::back_inserter(surround_units), make_surround_unit);

        for (const auto& unit : get_units<model::Minion>(context.cache())) {
            if (unit.second.value().getFaction() == model::FACTION_NEUTRAL && unit.second.is_active(context.world().getTickIndex())) {
                surround_units.push_back(make_enemy_surround_unit(unit.second.value()));
            } else {
                surround_units.push_back(make_surround_unit(&unit.second.value()));
            }
        }
    }

    double operator ()(const Point& position) const {
        context.check_timeout(__PRETTY_FUNCTION__, __FILE__, __LINE__);

        const auto units_danger_penalty = get_units_danger_penalty(position) * UNITS_DANGER_PENALTY_WEIGHT;
        const auto units_collision_penalty = get_units_collision_penalty(position) * UNITS_COLLISION_PENALTY_WEIGHT;
        const auto bonuses_penalty = get_bonuses_penalty(position) * BONUSES_PENALTY_WEIGHT;
        const auto projectiles_penalty = get_projectiles_penalty(position) * PROJECTILE_PENALTY_WEIGHT;
        const auto friendly_fire_penalty = get_friendly_fire_penalty(position) * FRIENDLY_FIRE_PENALTY_WEIGHT;
        const auto target_penalty = get_target_penalty(position) * TARGET_PENALTY_WEIGHT;
        const auto borders_penalty = get_borders_penalty(position) * BORDERS_PENALTY_WEIGHT;
        const auto friend_wizards_distance_penalty = get_friend_wizards_distance_penalty(position) * FRIEND_WIZARDS_DISTANCE_PENALTY_WEIGHT;
        const auto surround_penalty = get_surround_penalty(position) * SURROUND_PENALTY_WEIGHT;

        const auto max_penalty = std::max({
            units_danger_penalty,
            units_collision_penalty,
            bonuses_penalty,
            projectiles_penalty,
            friendly_fire_penalty,
            target_penalty,
            borders_penalty,
            friend_wizards_distance_penalty,
            surround_penalty,
        });

        const auto elimination_score = get_elimination_score(position) * ELIMINATION_SCORE_WEIGHT;

        return max_penalty - elimination_score;
    }

    double get_borders_penalty(const Point& position) const {
        const auto left = get_borders_factor(position.x());
        const auto right = get_borders_factor(context.game().getMapSize() - position.x());
        const auto top = get_borders_factor(position.y());
        const auto bottom = get_borders_factor(context.game().getMapSize() - position.y());
        return std::max({left, right, top, bottom});
    }

    double get_projectiles_penalty(const Point& position) const {
        const auto& projectiles = get_units<model::Projectile>(context.cache());
        return std::accumulate(projectiles.begin(), projectiles.end(), - std::numeric_limits<double>::max(),
            [&] (auto max, auto v) { return std::max(max, this->get_projectile_penalty(v.second, position)); });
    }

    double get_elimination_score(const Point& position) const {
        const auto get_sum_elimination_score = [&] (const auto& units) {
            return std::accumulate(units.begin(), units.end(), 0.0,
                [&] (auto sum, const auto& v) { return sum + this->get_elimination_score(v.second, position); });
        };

        const auto buildings_score = get_sum_elimination_score(get_units<model::Building>(context.cache()));
        const auto minions_score = get_sum_elimination_score(get_units<model::Minion>(context.cache()));
        const auto wizards_score = get_sum_elimination_score(get_units<model::Wizard>(context.cache()));

        return buildings_score + minions_score + wizards_score;
    }

    double get_sum_damage_to_me(const Point& position) const {
        const auto add_damage = [&] (auto sum, auto v) { return sum + this->get_unit_current_damage(*v, position); };

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
        const auto buildings_penalty = get_units_danger_penalty(enemy_buildings, position, sum_damage_to_me);
        const auto minions_penalty = get_units_danger_penalty(enemy_minions, position, sum_damage_to_me);
        const auto wizards_penalty = get_units_danger_penalty(enemy_wizards, position, sum_damage_to_me);
        return std::max({buildings_penalty, minions_penalty, wizards_penalty});
    }

    double get_units_collision_penalty(const Point& position) const {
        const auto buildings_penalty = get_units_collision_penalty(buildings, position);
        const auto minions_penalty = get_units_collision_penalty(minions, position);
        const auto trees_penalty = get_units_collision_penalty(trees, position);
        const auto wizards_penalty = get_units_collision_penalty(wizards, position);
        return std::max({buildings_penalty, minions_penalty, trees_penalty, wizards_penalty});
    }

    double get_bonuses_penalty(const Point& position) const {
        return std::accumulate(bonuses.begin(), bonuses.end(), - std::numeric_limits<double>::max(),
            [&] (auto max, auto v) { return std::max(max, this->get_bonus_penalty(*v, position)); });
    }

    double get_friendly_fire_penalty(const Point& position) const {
        const auto buildings_penalty = get_friendly_fire_penalty(friend_buildings, position);
        const auto trees_penalty = get_friendly_fire_penalty(trees, position);
        return std::max(buildings_penalty, trees_penalty);
    }

    double get_target_penalty(const Point& position) const {
        if (target) {
            return get_target_penalty(*target, position);
        } else {
            return - std::numeric_limits<double>::max();
        }
    }

    double get_friend_wizards_distance_penalty(const Point& position) const {
        if (friend_wizards.empty() || !context.game().isRawMessagesEnabled()) {
            return - std::numeric_limits<double>::max();
        }

        const auto closest = std::min_element(friend_wizards.begin(), friend_wizards.end(),
            [&] (auto lhs, auto rhs) {
                return get_position(*lhs).distance(position) < get_position(*rhs).distance(position);
            });
        const auto min_distance = get_position(**closest).distance(position);

        return line_factor(min_distance, 2 * context.game().getWizardRadius(), max_distance);
    }

    double get_surround_penalty(const Point& position) const {
        const auto borders_penalty = get_surround_penalty_by_borders(position);

        if (surround_units.size() < 2) {
            return borders_penalty;
        }

        const auto units_penalty = cross_product(surround_units.begin(), surround_units.end() - 1,
            surround_units.begin() + 1, surround_units.end(),
            - std::numeric_limits<double>::max(), [] (auto lhs, auto rhs) { return std::max(lhs, rhs); },
            [&] (const auto& lhs, const auto& rhs) { return this->get_surround_penalty(lhs, rhs, position); });

        const auto units_and_borders_penalty = std::accumulate(surround_units.begin(), surround_units.end(),
            - std::numeric_limits<double>::max(),
            [&] (auto max, const auto& unit) { return std::max(max, this->get_surround_penalty_by_borders(unit, position)); });

        return std::max({borders_penalty, units_penalty, units_and_borders_penalty});
    }

private:
    struct SurroundUnit {
        Point position;
        double influence_radius;
    };

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
    std::vector<const model::Minion*> friend_minions;
    std::vector<SurroundUnit> surround_units;

    double get_bonus_penalty(const model::Bonus& unit, const Point& position) const {
        const auto distance = position.distance(get_position(unit));
        const auto has_nearest_friend = friend_wizards.end() != std::find_if(friend_wizards.begin(), friend_wizards.end(),
                     [&] (auto v) { return get_position(*v).distance(get_position(unit)) < distance; });

        if (has_nearest_friend) {
            return - std::numeric_limits<double>::max();
        }

        return line_factor(distance, 0, max_distance);
    }

    double get_projectile_penalty(const CachedUnit<model::Projectile>& cached_unit, const Point& position) const {
        const auto& unit = cached_unit.value();

        if (unit.getFaction() == context.self().getFaction()) {
            return - std::numeric_limits<double>::max();
        }

        const auto trajectory = get_projectile_trajectory(cached_unit);
        const auto lethal_area = get_projectile_lethal_area(unit.getType());
        const auto nearest = trajectory.nearest(position);
        const auto has_point = trajectory.has_point(nearest);
        const auto distance = has_point ? nearest.distance(position)
                                        : std::min(trajectory.begin().distance(position), trajectory.end().distance(position));
        const auto safe_distance = lethal_area + context.self().getRadius() + 1;

        if (distance < safe_distance) {
            return 1 + 0.1 * line_factor(distance, safe_distance, 0);
        } else {
            return line_factor(distance, 2 * safe_distance, safe_distance);
        }
    }

    Line get_projectile_trajectory(const CachedUnit<model::Projectile>& cached_unit) const {
        const GetProjectileTrajectory impl {context};
        return impl(cached_unit);
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
        if (!target || &unit == target) {
            return - std::numeric_limits<double>::max();
        }

        const auto target_position = get_position(*target);
        const auto unit_position = get_position(unit);
        const auto has_intersection = Circle(unit_position, unit.getRadius())
                .has_intersection(Circle(position, context.self().getRadius()), target_position);

        if (!has_intersection) {
            return - std::numeric_limits<double>::max();
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
        const auto factor = bounded_line_factor(-mean_life_change_speed * 30, 0, unit.value().getLife());

        if (distance <= context.game().getScoreGainRange() - context.self().getRadius()) {
            return factor * (1 + 0.1 * bounded_line_factor(distance, context.game().getScoreGainRange()- context.self().getRadius(), 0));
        } else {
            return factor * bounded_line_factor(distance, context.game().getScoreGainRange(), context.game().getScoreGainRange() - context.self().getRadius());
        }
    }

    double get_borders_factor(double distance) const {
        return line_factor(distance, 2 * context.self().getRadius() + 1, context.self().getRadius() + 1);
    }

    template <class Unit>
    double get_unit_current_damage(const Unit& unit, const Point& position) const {
        const GetCurrentDamage get_current_damage {context};
        const ReduceDamage reduce_damage {context};
        const auto current_damage = get_current_damage(unit, position);
        const auto reduced_damage = reduce_damage(context.self(), current_damage);

        if (context.self().getLife() <= 3.0 * context.self().getMaxLife() / 4) {
            return reduced_damage.sum();
        }

        const auto unit_position = get_position(unit);
        const auto distance = position.distance(unit_position);
        const auto is_nearest = [&] (auto v) { return get_position(*v).distance(unit_position) < distance; };
        const auto nearest_minions_count = std::count_if(friend_minions.begin(), friend_minions.end(), is_nearest);
        const auto nearest_wizards_count = std::count_if(friend_wizards.begin(), friend_wizards.end(), is_nearest);

        return reduced_damage.sum() / (1 + nearest_minions_count + std::sqrt(nearest_wizards_count));
    }

    template <class Unit>
    double get_units_danger_penalty(const std::vector<const Unit*>& units, const Point& position, double sum_damage_to_me) const {
        const GetUnitDangerPenalty get_unit_danger_penalty {context, friend_units};
        return std::accumulate(units.begin(), units.end(), - std::numeric_limits<double>::max(),
            [&] (auto max, auto v) { return std::max(max, get_unit_danger_penalty(*v, position, sum_damage_to_me)); });
    }

    template <class Unit>
    double get_units_collision_penalty(const std::vector<const Unit*>& units, const Point& position) const {
        const GetUnitIntersectionPenalty get_unit_collision_penalty {context};
        return std::accumulate(units.begin(), units.end(), - std::numeric_limits<double>::max(),
           [&] (auto max, auto v) { return std::max(max, get_unit_collision_penalty(*v, position)); });
    }

    template <class Unit>
    double get_friendly_fire_penalty(const std::vector<const Unit*>& units, const Point& position) const {
        return std::accumulate(units.begin(), units.end(), - std::numeric_limits<double>::max(),
            [&] (auto max, auto v) { return std::max(max, this->get_friendly_fire_penalty(*v, position)); });
    }

    template <class Unit>
    double get_target_penalty(const Unit& unit, const Point& position) const {
        const GetMaxDamage get_max_damage {context};
        const auto current_distance = position.distance(get_position(unit));
        const auto future_distance = position.distance(get_position(unit) + get_speed(unit));
        const auto distance = std::max(current_distance, future_distance);
        const auto range = get_target_range(unit);

        if (distance <= range) {
            return line_factor(distance, range, 0);
        }

        const auto ticks_to_action = get_max_damage.next_attack_action(context.self(), distance).second;
        const auto ticks_factor = bounded_line_factor(ticks_to_action, context.game().getWizardActionCooldownTicks(), 0);
        const auto my_life_factor = double(context.self().getLife()) / double(context.self().getMaxLife());
        const auto distance_factor = line_factor(distance, 0, range) - 1;
        const auto unit_life_factor = 1 + bounded_line_factor(unit.getLife(), 2 * get_max_damage(context.self(), distance).sum(), 0);
        return distance_factor * ticks_factor * my_life_factor * unit_life_factor;
    }

    double get_target_penalty(const model::Bonus& unit, const Point& position) const {
        return get_bonus_penalty(unit, position);
    }

    double get_target_penalty(const model::LivingUnit&, const Point&) const {
        return 0;
    }

    template <class Unit>
    double get_target_range(const Unit& unit) const {
        const auto& cached_unit = get_units<Unit>(context.cache()).at(unit.getId());
        const auto unit_mean_life_change_speed = cached_unit.mean_life_change_speed();
        const auto my_mean_life_change_speed = context.cached_self().mean_life_change_speed();

        if (unit_mean_life_change_speed < 0 && my_mean_life_change_speed < 0) {
            const auto unit_ticks_to_die = - unit.getLife() / unit_mean_life_change_speed;
            const auto my_ticks_to_die = - context.self().getLife() / my_mean_life_change_speed;
            return unit_ticks_to_die < my_ticks_to_die
                    ? context.game().getStaffRange() + unit.getRadius() - 1
                    : context.self().getCastRange() - 1;
        } else if (unit_mean_life_change_speed < 0 || unit.getLife() < context.self().getLife()) {
            return context.game().getStaffRange() + unit.getRadius() - 1;
        } else {
            return context.self().getCastRange() - 1;
        }
    }

    double get_surround_penalty_by_borders(const SurroundUnit& unit, const Point& position) const {
        const Point left(0, unit.position.y());
        const Point right(context.world().getWidth(), unit.position.y());
        const Point top(unit.position.x(), 0);
        const Point bottom(unit.position.x(), context.world().getHeight());

        const auto borders = {left, right, top, bottom};
        const auto my_position = get_position(context.self());

        return std::accumulate(borders.begin(), borders.end(), - std::numeric_limits<double>::max(),
            [&] (auto max, const auto& border) {
                if (border.distance(my_position) <= max_distance) {
                    return std::max(max, this->get_surround_penalty(SurroundUnit {border, context.game().getWizardCastRange() * 0.5}, unit, position));
                } else {
                    return max;
                }
            });
    }

    double get_surround_penalty_by_borders(const Point& position) const {
        const std::array<SurroundUnit, 4> borders = {{
               SurroundUnit {Point(0, position.y()), context.game().getWizardCastRange() / 3},
               SurroundUnit {Point(context.world().getWidth(), position.y()), context.game().getWizardCastRange() / 3},
               SurroundUnit {Point(position.x(), 0), context.game().getWizardCastRange() / 3},
               SurroundUnit {Point(position.x(), context.world().getHeight()), context.game().getWizardCastRange() / 3},
        }};

        return cross_product(borders.begin(), borders.end() - 1, borders.begin() + 1, borders.end(),
              - std::numeric_limits<double>::max(), [] (auto lhs, auto rhs) { return std::max(lhs, rhs); },
              [&] (const auto& lhs, const auto& rhs) { return this->get_surround_penalty(lhs, rhs, position); });
    }

    double get_surround_penalty(const SurroundUnit& lhs, const SurroundUnit& rhs, const Point& position) const {
        const auto diameter = lhs.position - rhs.position;
        const auto units_distance = diameter.norm();
        const auto max_distance = lhs.influence_radius + rhs.influence_radius;

        if (units_distance >= max_distance) {
            return - std::numeric_limits<double>::max();
        }

        const auto center = rhs.position + 0.5 * diameter;
        const auto distance = center.distance(position);

        if (distance < units_distance) {
            return 1 + 0.1 * line_factor(distance, units_distance, 0);
        } else {
            return line_factor(distance, max_distance, units_distance);
        }
    }

    template <class Unit>
    SurroundUnit make_enemy_surround_unit(const Unit& unit) const {
        return SurroundUnit {get_position(unit), context.game().getStaffRange() + context.self().getRadius()};
    }
};

template <class TargetUnitT>
class GetOptimalPosition {
public:
    using TargetUnit = TargetUnitT;

    Point operator ()(const Context& context) const {
        const GetPositionPenalty<TargetUnit> get_position_penalty(context, target_, max_distance_);
        if (points_) {
            return minimize(context,
                [&] (const Point& point) {
                    const auto result = get_position_penalty(point);
                    points_->emplace_back(point, result);
                    return result;
                });
        } else {
            return minimize(context, get_position_penalty);
        }
    }

    GetOptimalPosition& target(const TargetUnit* value) {
        target_ = value;
        return *this;
    }

    GetOptimalPosition& max_distance(double value) {
        max_distance_ = value;
        return *this;
    }

    GetOptimalPosition& precision(double value) {
        precision_ = value;
        return *this;
    }

    GetOptimalPosition& max_function_calls(long value) {
        max_function_calls_ = value;
        return *this;
    }

    GetOptimalPosition& points(std::vector<std::pair<Point, double>>* value) {
        points_ = value;
        return *this;
    }

private:
    const TargetUnit* target_ = nullptr;
    double max_distance_ = std::numeric_limits<double>::max();
    double precision_ = 1e-3;
    long max_function_calls_ = std::numeric_limits<long>::max();
    std::vector<std::pair<Point, double>>* points_ = nullptr;

    template <class Function>
    Point minimize(const Context& context, const Function& function) const {
        return Minimize()
                .initial_trust_region_radius(precision_)
                .max_function_calls_count(max_function_calls_)
                .lower_bound(Point(context.self().getRadius() + 1, context.self().getRadius() + 1))
                .upper_bound(Point(context.world().getWidth() - context.self().getRadius() - 1, context.world().getHeight() - context.self().getRadius() - 1))
                (get_position(context.self()), function).second;
    }
};

}
