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

bool is_enemy(const model::Unit& unit, model::Faction my_faction);

bool is_friend(const model::Unit& unit, model::Faction my_faction, UnitId my_id);
bool is_friend(const model::Wizard& unit, model::Faction my_faction, UnitId my_id);

bool is_me(const model::Wizard& unit);
bool is_me(const model::Unit&);

double get_distance_penalty(double value, double safe);

Point get_optimal_position(const Context& context, const Target& target, double max_distance,
                           std::size_t initial_points_count, int max_function_calls);

template <class T>
std::vector<const T*> filter_friends(const std::vector<const T*>& units, model::Faction my_faction, UnitId my_id) {
    return filter_units(units, [&] (const auto& v) { return is_friend(v, my_faction, my_id); });
}

template <class T>
std::vector<const T*> filter_friends(const std::vector<T>& units, model::Faction my_faction, UnitId my_id) {
    return filter_units(units, [&] (const auto& v) { return is_friend(v, my_faction, my_id); });
}

struct GetAttackRange {
    const Context& context;

    double operator ()(const model::Unit&) const;
    double operator ()(const model::Building& unit) const;
    double operator ()(const model::Minion& unit) const;
    double operator ()(const model::Wizard& unit) const;
};

struct GetUnitIntersectionPenalty {
    const Context& context;

    double operator ()(const model::CircularUnit& unit, const Point& position) const;
    double operator ()(const model::Tree& unit, const Point& position) const;
};

struct GetRangedDamage {
    const Context& context;

    template <class Unit>
    double operator ()(const Unit& unit, const Point& position) const {
        const GetAttackRange get_attack_range {context};
        const GetDamage get_damage {context};
        const auto attack_range = get_attack_range(unit) + context.self().getRadius();
        const auto current_distance = position.distance(get_position(unit));
        const auto future_distance = position.distance(get_position(unit) + get_speed(unit));
        const auto factor = attack_range >= current_distance || attack_range >= future_distance
                ? 1.0
                : get_distance_penalty(0.5 * (current_distance + future_distance) - attack_range, 0.5 * attack_range);
        return factor * get_damage(unit);
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
        return get_ranged_damage(unit, position) * get_attack_ability(unit);
    }
};

struct GetUnitDangerPenalty {
    const Context& context;
    const std::vector<const model::CircularUnit*>& friend_units;

    template <class T>
    double operator ()(const T& unit, const Point& position, double damage_factor, double sum_enemy_damage) const {
        return is_enemy(unit, context.self().getFaction()) ? get_common(unit, position, damage_factor, sum_enemy_damage) : 0;
    }

    double operator ()(const model::Minion& unit, const Point& position, double damage_factor, double sum_enemy_damage) const;

    template <class T>
    double get_common(const T& unit, const Point& position, double damage_factor, double sum_enemy_damage) const {
        const GetDamage get_damage {context};
        const GetAttackRange get_attack_range {context};
        const GetCurrentDamage get_current_damage {context};
        double distance_factor;
        if (sum_enemy_damage + damage_factor * get_current_damage(unit, position) > 2 * context.self().getLife()) {
            distance_factor = 1.0;
        } else {
            distance_factor = damage_factor * get_current_damage(unit, position) / get_damage(unit);
        }
        const double safe_distance = std::max(
            context.self().getCastRange() + context.game().getMagicMissileRadius(),
            distance_factor * (get_attack_range(unit) + 2 * context.self().getRadius())
        );
        return get_distance_penalty(position.distance(get_position(unit)), safe_distance);
    }
};

template <class T>
struct IsTarget {
    const T* target;

    template <class V>
    typename std::enable_if<std::is_same<T, V>::value, bool>::type operator ()(const V& unit) const {
        return target ? unit.getId() == target->getId() : false;
    }

    template <class V>
    typename std::enable_if<!std::is_same<T, V>::value, bool>::type operator ()(const V&) const {
        return false;
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
        const IsTarget<T> is_target {target};

        const auto initial_filter = [&] (const auto& units) {
            return filter_units(units, [&] (const auto& unit) { return !is_me(unit) && !is_target(unit) && is_in_my_range(unit); });
        };

        buildings = initial_filter(get_units<model::Building>(context.cache()));
        minions = initial_filter(get_units<model::Minion>(context.cache()));
        wizards = initial_filter(get_units<model::Wizard>(context.cache()));
        trees = initial_filter(get_units<model::Tree>(context.cache()));
        projectiles = initial_filter(get_units<model::Projectile>(context.cache()));
        bonuses = initial_filter(get_units<model::Bonus>(context.cache()));

        const auto is_enemy_and_not_target = [&] (const auto& unit) {
            return !is_target(unit) && is_enemy(unit, context.self().getFaction());
        };

        enemy_wizards = filter_units(wizards, is_enemy_and_not_target);
        enemy_minions = filter_units(minions, is_enemy_and_not_target);
        enemy_buildings = filter_units(buildings, is_enemy_and_not_target);

        const auto friend_wizards = filter_friends(wizards, context.self().getFaction(), context.self().getId());
        const auto friend_minions = filter_friends(minions, context.self().getFaction(), context.self().getId());
        const auto friend_buildings = filter_friends(buildings, context.self().getFaction(), context.self().getId());

        friend_units.reserve(friend_wizards.size() + friend_minions.size() + friend_buildings.size());
        std::copy(friend_wizards.begin(), friend_wizards.end(), std::back_inserter(friend_units));
        std::copy(friend_minions.begin(), friend_minions.end(), std::back_inserter(friend_units));
        std::copy(friend_buildings.begin(), friend_buildings.end(), std::back_inserter(friend_units));

        damage_factor = 1.0 - is_shielded(context.self()) * context.game().getShieldedDirectDamageAbsorptionFactor();
        max_borders_penalty = double(bonuses.size() + buildings.size() + minions.size()
                                 + projectiles.size() + trees.size() + wizards.size());
    }

    double operator ()(const Point& position) const {
        context.check_timeout(__PRETTY_FUNCTION__, __FILE__, __LINE__);

        const GetUnitIntersectionPenalty get_unit_collision_penalty {context};
        const GetUnitDangerPenalty get_unit_danger_penalty {context, friend_units};
        const GetCurrentDamage get_current_damage {context};

        const bool is_out_of_borders = context.self().getRadius() >= position.x()
                || position.x() >= context.game().getMapSize() - context.self().getRadius()
                || context.self().getRadius() >= position.y()
                || position.y() >= context.game().getMapSize() - context.self().getRadius();

        if (is_out_of_borders) {
            return max_borders_penalty;
        }

        const double enemy_wizards_damage = std::accumulate(enemy_wizards.begin(), enemy_wizards.end(), 0.0,
            [&] (auto sum, const auto& v) { return sum + damage_factor * get_current_damage(*v, position); });

        const double enemy_minions_damage = std::accumulate(enemy_minions.begin(), enemy_minions.end(), 0.0,
            [&] (auto sum, const auto& v) { return sum + damage_factor * get_current_damage(*v, position); });

        const double enemy_buildings_damage = std::accumulate(enemy_buildings.begin(), enemy_buildings.end(), 0.0,
            [&] (auto sum, const auto& v) { return sum + damage_factor * get_current_damage(*v, position); });

        const double sum_enemy_damage = enemy_wizards_damage
                + enemy_minions_damage
                + enemy_buildings_damage;

        const auto get_sum_wizards_penalty = [&] (const auto& units, const Point& position) {
            return std::accumulate(units.begin(), units.end(), 0.0,
                [&] (auto sum, const model::Wizard* v) {
                    return sum + std::max(get_unit_collision_penalty(*v, position),
                                          get_unit_danger_penalty(*v, position, damage_factor, sum_enemy_damage));
                });
        };

        const auto get_sum_units_penalty = [&] (const auto& units, const Point& position) {
            return std::accumulate(units.begin(), units.end(), 0.0,
                [&] (auto sum, auto v) {
                    return sum + std::max(get_unit_collision_penalty(*v, position),
                                          get_unit_danger_penalty(*v, position, damage_factor, sum_enemy_damage));
                });
        };

        const auto get_sum_bonuses_penalty = [&] (const auto& units, const Point& position) {
            return std::accumulate(units.begin(), units.end(), 0.0,
                [&] (auto sum, auto v) {
                    return sum + this->get_bonus_penalty(*v, position);
                });
        };

        const auto get_sum_friendly_fire_penalty = [&] (const auto& units, const Point& position) {
            return std::accumulate(units.begin(), units.end(), 0.0,
                [&] (auto sum, auto v) {
                    return sum + this->get_friendly_fire_penalty(*v, position);
                });
        };

        const auto get_sum_projectiles_penalty = [&] (const auto& units, const Point& position) {
            return std::accumulate(units.begin(), units.end(), 0.0,
                [&] (auto sum, auto v) {
                    return sum + this->get_projectile_penalty(*v, position);
                });
        };

        double target_penalty = 0;

        if (target) {
            target_penalty = get_unit_danger_penalty(*target, position, damage_factor, sum_enemy_damage);

            if (!dynamic_cast<const model::Bonus*>(target)) {
                const auto current_distance = position.distance(get_position(*target));
                const auto future_distance = position.distance(get_position(*target) + get_speed(*target));
                const auto distance = 0.5 * (current_distance + future_distance);
                const auto range = context.self().getCastRange() + target->getRadius();
                const auto distance_penalty = 1.0 - get_distance_penalty(distance - range, context.self().getVisionRange());
                target_penalty = std::max(get_unit_danger_penalty(*target, position, damage_factor, sum_enemy_damage),
                                          distance_penalty);
            }
        }

        const auto except_borders = get_sum_units_penalty(buildings, position)
                + get_sum_units_penalty(minions, position)
                + get_sum_units_penalty(trees, position)
                + get_sum_wizards_penalty(wizards, position)
                + get_sum_bonuses_penalty(bonuses, position)
                + get_sum_projectiles_penalty(projectiles, position)
                + get_sum_friendly_fire_penalty(friend_units, position)
                + target_penalty;

        const auto borders_distance_penalty = get_border_distance_penalty(position.x(), max_borders_penalty - except_borders)
                + get_border_distance_penalty(context.game().getMapSize() - position.x(), max_borders_penalty - except_borders)
                + get_border_distance_penalty(position.y(), max_borders_penalty - except_borders)
                + get_border_distance_penalty(context.game().getMapSize() - position.y(), max_borders_penalty - except_borders);

        return except_borders + borders_distance_penalty;
    }

private:
    const Context& context;
    const Target* const target;
    const double max_distance;
    std::vector<const model::Bonus*> bonuses;
    std::vector<const model::Building*> buildings;
    std::vector<const model::Minion*> minions;
    std::vector<const model::Projectile*> projectiles;
    std::vector<const model::Tree*> trees;
    std::vector<const model::Wizard*> wizards;
    std::vector<const model::Wizard*> enemy_wizards;
    std::vector<const model::Minion*> enemy_minions;
    std::vector<const model::Building*> enemy_buildings;
    std::vector<const model::CircularUnit*> friend_units;
    double max_borders_penalty = std::numeric_limits<double>::max();
    double damage_factor = 1;

    double get_bonus_penalty(const model::Bonus& unit, const Point& position) const {
        const auto distance = position.distance(get_position(unit));
        return 1 - get_distance_penalty(distance, 2 * context.self().getVisionRange());
    }

    template <class Unit>
    double get_ranged_damage(const Unit& unit, const Point& position) const {
        const GetAttackRange get_attack_range {context};
        const GetDamage get_damage {context};
        const auto attack_range = get_attack_range(unit);
        const auto distance = position.distance(get_position(unit));
        const auto factor = attack_range >= distance ? 1 : get_distance_penalty(distance - attack_range, 0.5 * attack_range);
        return factor * get_damage(unit);
    }

    double get_projectile_penalty(const model::Projectile& unit, const Point& position) const {
        const auto unit_speed = get_speed(unit);
        const auto unit_position = get_position(unit);
        const auto safe_distance = 2 * context.self().getRadius() + unit.getRadius();
        const auto distance_to = Line(unit_position, unit_position + unit_speed).distance(position);
        return get_distance_penalty(distance_to, safe_distance);
    }

    double get_friendly_fire_penalty(const model::CircularUnit& unit, const Point& position) const {
        if (!target) {
            return 0.0;
        }

        const auto target_position = get_position(*target);
        const auto unit_position = get_position(unit);
        const auto has_intersection = Circle(unit_position, unit.getRadius())
                .has_intersection(Circle(get_position(context.self()), context.self().getRadius()), target_position);

        if (!has_intersection) {
            return 0.0;
        }

        const auto target_to_unit = unit_position - target_position;
        const auto tangent_cos = (unit.getRadius() + context.game().getMagicMissileRadius())
                / target_position.distance(unit_position);
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

    double get_border_distance_penalty(double distance, double max_penalty) const {
        const auto safe_distance = 4 * context.self().getRadius();
        if (distance < 0.5 * safe_distance) {
            return max_penalty * get_distance_penalty(distance, safe_distance);
        } else {
            return max_penalty * 0.5 * get_distance_penalty(distance, 2 * safe_distance);
        }
    }
};

template <class T>
Point get_optimal_position(const Context& context, const T* target, double max_distance,
                           std::size_t initial_points_count, int max_function_calls) {
    const GetPositionPenalty<T> get_position_penalty(context, target, max_distance);
    std::vector<std::pair<double, Point>> points;
    points.reserve(initial_points_count + 1);
    points.emplace_back(minimize(get_position_penalty, get_position(context.self()), max_function_calls));
    std::size_t step = 0;
    std::generate_n(std::back_inserter(points), initial_points_count - 1,
        [&] {
            context.check_timeout(__PRETTY_FUNCTION__, __FILE__, __LINE__);
            const auto angle = normalize_angle(2.0 * M_PI * double(step++) / double(initial_points_count));
            const auto initial = get_position(context.self()) + Point(1, 0).rotated(angle) * 0.25 * context.self().getVisionRange();
            return minimize(get_position_penalty, initial, max_function_calls);
        });
    return std::min_element(points.begin(), points.end(),
        [] (const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; })->second;
}

template <>
inline Point get_optimal_position(const Context& /*context*/, const model::Bonus* target, double /*max_distance*/,
                                  std::size_t /*initial_points_count*/, int /*max_function_calls*/) {
    return get_position(*target);
}

}

#endif
