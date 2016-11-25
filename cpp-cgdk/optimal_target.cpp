#include "optimal_target.hpp"

#include <algorithm>

namespace strategy {

Target get_optimal_target(const Context& context, double max_distance) {
    IsInMyRange is_in_my_range {context, max_distance};

    const auto is_enemy_and_in_my_range = [&] (const auto& unit) {
        return is_enemy(unit, context.self.getFaction()) && is_in_my_range(unit);
    };

    const auto enemy_wizards = filter_units(context.world.getWizards(), is_enemy_and_in_my_range);
    const auto enemy_minions = filter_units(context.world.getMinions(), is_enemy_and_in_my_range);
    const auto enemy_buildings = filter_units(context.world.getBuildings(), is_enemy_and_in_my_range);

    const auto bonuses = filter_units(context.world.getBuildings(), is_in_my_range);

    if (enemy_wizards.empty() && enemy_minions.empty() && enemy_wizards.empty() && bonuses.empty()) {
        const double factor = get_speed(context.self).norm() < 1 ? 2 : 1.3;
        const auto trees = filter_units(context.world.getTrees(),
            [&] (const auto& unit) {
                return get_position(context.self).distance(get_position(unit))
                        < factor * context.self.getRadius() + unit.getRadius();
            });
        if (trees.empty()) {
            return Target();
        }
        const auto target = std::min_element(trees.begin(), trees.end(),
            [] (auto lhs, auto rhs) { return lhs->getLife() < rhs->getLife(); });
        return *target;
    }

    if (!bonuses.empty()) {
        const auto target = std::min_element(bonuses.begin(), bonuses.end(),
            [&] (auto lhs, auto rhs) {
                return get_position(*lhs).distance(get_position(context.self))
                        < get_position(*rhs).distance(get_position(context.self));
            });
        return *target;
    }

    const GetDamage get_damage {context};

    const auto get_target_penalty = [&] (const auto& unit) {
        const auto distance = get_position(unit).distance(get_position(context.self));
        return distance <= 2 * unit.getRadius() + context.self.getVisionRange()
                ? distance * unit.getLife() / get_damage(context.self)
                : distance;
    };

    const auto get_with_min_penalty = [&] (const auto& units) {
        return std::min_element(units.begin(), units.end(),
            [&] (auto lhs, auto rhs) { return get_target_penalty(*lhs) < get_target_penalty(*rhs); });
    };

    IsInMyRange is_in_staff_range {context, context.game.getStaffRange()};

    if (!enemy_wizards.empty()) {
        const auto in_staff_range = filter_units(enemy_wizards, is_in_staff_range);
        if (!in_staff_range.empty()) {
            return *get_with_min_penalty(in_staff_range);
        }
    }

    if (!enemy_minions.empty()) {
        const auto in_staff_range = filter_units(enemy_minions, is_in_staff_range);
        if (!in_staff_range.empty()) {
            return *get_with_min_penalty(in_staff_range);
        }
    }

    if (!enemy_wizards.empty()) {
        return *get_with_min_penalty(enemy_wizards);
    }

    if (!enemy_minions.empty()) {
        return *get_with_min_penalty(enemy_minions);
    }

    if (!enemy_buildings.empty()) {
        return *get_with_min_penalty(enemy_buildings);
    }

    return Target();
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

}
