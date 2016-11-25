#include "optimal_position.hpp"
#include "minimize.hpp"
#include "circle.hpp"

#include <stdexcept>
#include <algorithm>
#include <numeric>
#include <type_traits>

namespace strategy {

const model::LivingUnit* get_target(const Context& /*context*/) {
    return nullptr;
}

bool is_with_status(const model::LivingUnit& unit, model::StatusType status) {
    return unit.getStatuses().end() != std::find_if(
                unit.getStatuses().begin(), unit.getStatuses().end(),
                [&] (const model::Status& v) { return v.getType() == status; });
}

bool is_empowered(const model::LivingUnit& unit) {
    return is_with_status(unit, model::STATUS_EMPOWERED);
}

bool is_shielded(const model::LivingUnit& unit) {
    return is_with_status(unit, model::STATUS_SHIELDED);
}

bool is_enemy(const model::Unit& unit, model::Faction my_faction) {
    return unit.getFaction() != my_faction
            && unit.getFaction() != model::FACTION_NEUTRAL
            && unit.getFaction() != model::FACTION_OTHER;
}

bool is_friend(const model::Unit& unit, model::Faction my_faction, UnitId) {
    return unit.getFaction() == my_faction;
}

bool is_friend(const model::Wizard& unit, model::Faction my_faction, UnitId my_id) {
    return unit.getFaction() == my_faction && unit.getId() != my_id;
}

struct GetDamage {
    const Context& context;

    double operator ()(const model::Unit&) const {
        return 0.0;
    }

    double operator ()(const model::Building& unit) const {
        return get_factor(unit) * unit.getDamage();
    }

    double operator ()(const model::Minion& unit) const {
        return get_factor(unit) * unit.getDamage();
    }

    double operator ()(const model::Wizard& unit) const {
        return get_factor(unit) * context.game.getMagicMissileDirectDamage();
    }

    double get_factor(const model::LivingUnit& unit) const {
        return 1 + is_empowered(unit) * context.game.getEmpoweredDamageFactor();
    }
};

struct GetAttackRange {
    const Context& context;

    double operator ()(const model::Unit&) const {
        return 0.0;
    }

    double operator ()(const model::Building& unit) const {
        return context.self.getRadius() + unit.getAttackRange();
    }

    double operator ()(const model::Minion& unit) const {
        switch (unit.getType()) {
            case model::_MINION_UNKNOWN_:
                break;
            case model::MINION_ORC_WOODCUTTER:
                return context.self.getRadius() + context.game.getOrcWoodcutterAttackRange();
            case model::MINION_FETISH_BLOWDART:
                return context.self.getRadius() + context.game.getFetishBlowdartAttackRange()
                        + context.game.getDartRadius();
            case model::_MINION_COUNT_:
                break;
        }
        throw std::logic_error("Invalid minion type: " + std::to_string(unit.getType()));
    }

    double operator ()(const model::Wizard& unit) const {
        return context.self.getRadius() + unit.getCastRange() + context.game.getMagicMissileRadius();
    }
};

double get_distance_penalty(double value, double safe) {
    return std::min(1.0, std::max(0.0, (safe - value) / safe));
}

struct GetUnitIntersectionPenalty {
    const Context& context;

    double operator ()(const model::CircularUnit& unit) const {
        return get_distance_penalty(get_position(context.self).distance(get_position(unit)),
                                    1.1 * context.self.getRadius() + unit.getRadius());
    }

    double operator ()(const model::Tree& unit) const {
        return get_distance_penalty(get_position(context.self).distance(get_position(unit)),
                                    2 * context.self.getRadius() + 2 * unit.getRadius());
    }
};

template <class GetRangedDamage>
struct GetUnitDangerPenalty {
    const Context& context;
    const GetDamage& get_damage;
    const GetAttackRange& get_attack_range;
    const GetRangedDamage& get_ranged_damage;
    const std::vector<const model::CircularUnit*>& friend_units;

    template <class T>
    double operator ()(const T& unit, const Point& position, double damage_factor, double sum_enemy_damage) const {
        return is_enemy(unit, context.self.getFaction()) ? get_common(unit, position, damage_factor, sum_enemy_damage) : 0;
    }

    double operator ()(const model::Minion& unit, const Point& position, double damage_factor, double sum_enemy_damage) const {
        if (!is_enemy(unit, context.self.getFaction())) {
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
        if (unit_position.distance(get_position(**nearest_friend)) < unit_position.distance(get_position(context.self))) {
            return 0;
        }
        return get_common(unit, position, damage_factor, sum_enemy_damage);
    }

    template <class T>
    double get_common(const T& unit, const Point& position, double damage_factor, double sum_enemy_damage) const {
        const double add_damage = damage_factor * (get_damage(unit) - get_ranged_damage(unit, position));
        const double safe_distance = std::max(
                context.self.getCastRange() + context.game.getMagicMissileRadius(),
                (get_attack_range(unit) + 2 * context.self.getRadius())
                    * std::min(1.0, 2 * (sum_enemy_damage + add_damage) / context.self.getLife())
        );
        return get_distance_penalty(get_position(context.self).distance(get_position(unit)), safe_distance);
    }
};


Point get_optimal_position(const Context& context, const model::LivingUnit* target) {
    const auto is_target = [&] (const auto& unit) {
        using Type = typename std::decay<decltype(unit)>::type;
        if (const auto casted = dynamic_cast<const Type*>(target)) {
            return casted->getId() == unit.getId();
        }
        return false;
    };

    const auto is_enemy_and_not_target = [&] (const auto& unit) {
        return !is_target(unit) && is_enemy(unit, context.self.getFaction());
    };

    const auto enemy_wizards = filter_units(context.world.getWizards(), is_enemy_and_not_target);
    const auto enemy_minions = filter_units(context.world.getMinions(), is_enemy_and_not_target);
    const auto enemy_buildings = filter_units(context.world.getBuildings(), is_enemy_and_not_target);

    const auto friend_wizards = filter_friends(context.world.getWizards(), context.self.getFaction(), context.self.getId());
    const auto friend_minions = filter_friends(context.world.getMinions(), context.self.getFaction(), context.self.getId());
    const auto friend_buildings = filter_friends(context.world.getBuildings(), context.self.getFaction(), context.self.getId());

    std::vector<const model::CircularUnit*> friend_units;
    friend_units.reserve(friend_wizards.size() + friend_minions.size() + friend_buildings.size());
    std::copy(friend_wizards.begin(), friend_wizards.end(), std::back_inserter(friend_units));
    std::copy(friend_minions.begin(), friend_minions.end(), std::back_inserter(friend_units));
    std::copy(friend_buildings.begin(), friend_buildings.end(), std::back_inserter(friend_units));

    const GetAttackRange get_attack_range {context};
    const GetDamage get_damage {context};
    const GetUnitIntersectionPenalty get_unit_collision_penalty {context};

    const auto get_ranged_damage = [&] (const auto& unit, const Point& position) {
        const auto attack_range = get_attack_range(unit);
        const auto distance = position.distance(get_position(unit));
        const auto factor = attack_range >= distance ? 1 : get_distance_penalty(distance - attack_range, 0.5 * attack_range);
        return factor * get_damage(unit);
    };

    const GetUnitDangerPenalty<decltype(get_ranged_damage)> get_unit_danger_penalty {
        context,
        get_damage,
        get_attack_range,
        get_ranged_damage,
        friend_units,
    };

    const auto get_bonus_penalty = [&] (const model::Bonus& unit, const Point& position) {
        const auto unit_position = get_position(unit);
        return 1 - get_distance_penalty(position.distance(unit_position),
                                        get_position(context.self).distance(unit_position) + context.self.getRadius());
    };

    const auto get_projectile_penalty = [&] (const model::Projectile& unit, const Point& position) {
        const auto unit_speed = get_speed(unit);
        const auto unit_position = get_position(unit);
        const auto safe_distance = 2 * context.self.getRadius() + unit.getRadius();
        const auto distance_to = Line(unit_position, unit_position + unit_speed).distance(position);
        return get_distance_penalty(distance_to, safe_distance);
    };

    const auto my_position = get_position(context.self);
    const auto damage_factor = 1.0 - is_shielded(context.self) * context.game.getShieldedDirectDamageAbsorptionFactor();

    const auto get_friendly_fire_penalty = [&] (const model::CircularUnit& unit, const Point& position) {
        if (!target) {
            return 0.0;
        }

        const auto target_position = get_position(*target);
        const auto unit_position = get_position(unit);
        const auto has_intersection = Circle(unit_position, unit.getRadius())
                .has_intersection_with_moving_circle(Circle(my_position, context.self.getRadius()),
                                                     target_position);

        if (!has_intersection) {
            return 0.0;
        }

        const auto target_to_unit = unit_position - target_position;
        const auto tangent_cos = (unit.getRadius() + context.game.getMagicMissileRadius())
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
    };

    const auto get_position_penalty = [&] (const Point& position) {
        const bool borders_penalty = context.self.getRadius() < position.x()
                && position.x() < context.game.getMapSize() - context.self.getRadius()
                && context.self.getRadius() < position.y()
                && position.y() < context.game.getMapSize() - context.self.getRadius();

        if (borders_penalty) {
            return double(context.world.getBonuses().size()
                    + context.world.getBuildings().size()
                    + context.world.getMinions().size()
                    + context.world.getProjectiles().size()
                    + context.world.getTrees().size()
                    + context.world.getWizards().size());
        }

        const double enemy_wizards_damage = std::accumulate(
                    enemy_wizards.begin(), enemy_wizards.end(), 0.0,
                    [&] (auto sum, const auto& v) { return sum + damage_factor * get_ranged_damage(*v, position); });

        const double enemy_minions_damage = std::accumulate(
                    enemy_minions.begin(), enemy_minions.end(), 0.0,
                    [&] (auto sum, const auto& v) { return sum + damage_factor * get_ranged_damage(*v, position); });

        const double enemy_buildings_damage = std::accumulate(
                    enemy_buildings.begin(), enemy_buildings.end(), 0.0,
                    [&] (auto sum, const auto& v) { return sum + damage_factor * get_ranged_damage(*v, position); });

        const double sum_enemy_damage = enemy_wizards_damage + enemy_minions_damage + enemy_buildings_damage;

        const auto get_sum_units_penalty = [&] (const auto& units, const Point& position) {
            return std::accumulate(units.begin(), units.end(), 0.0,
                [&] (auto sum, const auto& v) {
                    return sum + std::max(get_unit_collision_penalty(v),
                                          get_unit_danger_penalty(v, position, damage_factor, sum_enemy_damage));
                });
        };

        const auto get_sum_bonuses_penalty = [&] (const auto& units, const Point& position) {
            return std::accumulate(units.begin(), units.end(), 0.0,
                [&] (auto sum, const auto& v) {
                    return sum + get_bonus_penalty(v, position);
                });
        };

        const auto get_sum_friendly_fire_penalty = [&] (const auto& units, const Point& position) {
            return std::accumulate(units.begin(), units.end(), 0.0,
                [&] (auto sum, auto v) {
                    return sum + get_friendly_fire_penalty(*v, position);
                });
        };

        const auto get_sum_projectiles_penalty = [&] (const auto& units, const Point& position) {
            return std::accumulate(units.begin(), units.end(), 0.0,
                [&] (auto sum, const auto& v) {
                    return sum + get_projectile_penalty(v, position);
                });
        };

        double target_distance_penalty = 0;

        if (target) {
            if (const auto bonus = dynamic_cast<const model::Bonus*>(target)) {
                target_distance_penalty = get_bonus_penalty(*bonus, position);
            } else {
                const auto max_cast_range = context.self.getCastRange() + context.game.getMagicMissileRadius();
                const auto distance = position.distance(get_position(*target));
                double distance_penalty = 0;
                if (distance > max_cast_range) {
                    const auto safe_distance = std::max(context.self.getCastRange(), context.game.getMapSize());
                    distance_penalty = 1 - get_distance_penalty(distance - max_cast_range, safe_distance);
                }
                target_distance_penalty = std::max(get_unit_danger_penalty(*target, position, damage_factor, sum_enemy_damage),
                                                   distance_penalty);
            }
        }

        return get_sum_units_penalty(context.world.getBuildings(), position)
                + get_sum_units_penalty(context.world.getMinions(), position)
                + get_sum_units_penalty(context.world.getTrees(), position)
                + get_sum_units_penalty(context.world.getWizards(), position)
                + get_sum_bonuses_penalty(context.world.getBonuses(), position)
                + get_sum_projectiles_penalty(context.world.getProjectiles(), position)
                + get_sum_friendly_fire_penalty(friend_units, position)
                + target_distance_penalty;
    };

    return minimize(get_position_penalty, my_position, 100);
}

}
