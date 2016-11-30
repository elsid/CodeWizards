#include "helpers.hpp"

namespace strategy {

bool has_skill(const model::Wizard& unit, model::SkillType skill) {
    return unit.getSkills().end() != std::find(unit.getSkills().begin(), unit.getSkills().end(), skill);
}

Point get_position(const model::Unit& unit) {
    return Point(unit.getX(), unit.getY());
}

Point get_speed(const model::Unit& unit) {
    return Point(unit.getSpeedX(), unit.getSpeedY());
}

int get_hastened_remaining_ticks(const model::LivingUnit& unit) {
    const auto hastend = find_status(unit.getStatuses(), model::STATUS_HASTENED);
    return hastend == unit.getStatuses().end() ? 0 : hastend->getRemainingDurationTicks();
}

double normalize_angle(double value) {
    if (value > M_PI) {
        return value - std::round(value * 0.5 * M_1_PI) * 2.0 * M_PI;
    }
    if (value < -M_PI) {
        return value + std::round(std::abs(value) * 0.5 * M_1_PI) * 2.0 * M_PI;
    }
    return value;
}

bool is_friend(const model::Unit& unit, model::Faction my_faction, UnitId) {
    return unit.getFaction() == my_faction;
}

bool is_friend(const model::Wizard& unit, model::Faction my_faction, UnitId my_id) {
    return unit.getFaction() == my_faction && unit.getId() != my_id;
}

bool is_me(const model::Wizard& unit) {
    return unit.isMe();
}

bool is_me(const model::Unit&) {
    return false;
}

std::vector<model::Status>::const_iterator find_status(const std::vector<model::Status>& statuses, model::StatusType status) {
    return std::find_if(statuses.begin(), statuses.end(),
                        [&] (const model::Status& v) { return v.getType() == status; });
}

bool is_with_status(const model::LivingUnit& unit, model::StatusType status) {
    return unit.getStatuses().end() != find_status(unit.getStatuses(), status);
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
