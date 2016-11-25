#include "optimal_position.hpp"

namespace strategy {

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

double get_distance_penalty(double value, double safe) {
    return std::min(1.0, std::max(0.0, (safe - value) / safe));
}

}
