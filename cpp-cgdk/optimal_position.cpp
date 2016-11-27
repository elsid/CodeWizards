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

Point get_optimal_position(const Context& context, const Target& target, double max_distance,
                           std::vector<std::pair<Point, double>>* points) {
    if (const auto unit = target.unit<model::Bonus>(context.cache())) {
        return get_optimal_position(context, unit, max_distance, points);
    } else if (const auto unit = target.unit<model::Building>(context.cache())) {
        return get_optimal_position(context, unit, max_distance, points);
    } else if (const auto unit = target.unit<model::Minion>(context.cache())) {
        return get_optimal_position(context, unit, max_distance, points);
    } else if (const auto unit = target.unit<model::Wizard>(context.cache())) {
        return get_optimal_position(context, unit, max_distance, points);
    } else if (const auto unit = target.unit<model::Tree>(context.cache())) {
        return get_optimal_position(context, unit, max_distance, points);
    }
    throw std::logic_error("Target has no value in " + std::string(__FUNCTION__));
}

bool is_me(const model::Wizard& unit) {
    return unit.isMe();
}

bool is_me(const model::Unit&) {
    return false;
}

}
