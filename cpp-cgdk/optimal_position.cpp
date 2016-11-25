#include "optimal_position.hpp"

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
    if (const auto unit = target.bonus()) {
        return get_optimal_position(context, unit, max_distance);
    } else if (const auto unit = target.building()) {
        return get_optimal_position(context, unit, max_distance);
    } else if (const auto unit = target.minion()) {
        return get_optimal_position(context, unit, max_distance);
    } else if (const auto unit = target.wizard()) {
        return get_optimal_position(context, unit, max_distance);
    } else if (const auto unit = target.tree()) {
        return get_optimal_position(context, unit, max_distance);
    }
    throw std::logic_error("Target has no value in " + std::string(__FUNCTION__));
}

}
