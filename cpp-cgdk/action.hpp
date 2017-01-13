#pragma once

#include "context.hpp"
#include "target.hpp"

namespace strategy {

class Action {
public:
    Action() = default;

    Action(double cast_angle, double min_cast_distance, double max_cast_distance)
            : cast_angle_(cast_angle), min_cast_distance_(min_cast_distance), max_cast_distance_(max_cast_distance) {}

    Action(UnitId status_target_id)
            : status_target_id_(status_target_id) {}

    double cast_angle() const {
        return cast_angle_;
    }

    double min_cast_distance() const {
        return min_cast_distance_;
    }

    double max_cast_distance() const {
        return max_cast_distance_;
    }

    UnitId status_target_id() const {
        return status_target_id_;
    }

private:
    double cast_angle_ = 0;
    double min_cast_distance_ = 0;
    double max_cast_distance_ = std::numeric_limits<double>::max();
    UnitId status_target_id_ = -1;
};

std::vector<model::ActionType> get_actions_by_priority_order(const Context& context, const Target& target);
std::pair<bool, Action> need_apply_action(const Context& context, const Target& target, model::ActionType type);
Point get_optimal_target_position(const model::Unit& unit);
Point get_optimal_target_position(const model::Wizard& unit);

}
