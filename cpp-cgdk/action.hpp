#pragma once

#include "context.hpp"
#include "target.hpp"

namespace strategy {

struct Action {
    double cast_angle = 0;
    double min_cast_distance = 0;
    double max_cast_distance = std::numeric_limits<double>::max();
};

std::vector<model::ActionType> get_actions_by_priority_order(const Context& context, const Target& target);
std::pair<bool, Action> need_apply_action(const Context& context, const Target& target, model::ActionType type);

}
