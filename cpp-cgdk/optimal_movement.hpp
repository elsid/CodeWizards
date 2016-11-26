#pragma once

#include "point.hpp"
#include "context.hpp"

#include <vector>

namespace strategy {

struct State {
    Point position;
    double angle;
};

struct Movement {
    double speed;
    double strafe_speed;
    double turn;
};

using States = std::vector<State>;
using Movements = std::vector<Movement>;

std::pair<States, Movements> get_optimal_movement(const Context& context, const Point& target, double step_size);

}
