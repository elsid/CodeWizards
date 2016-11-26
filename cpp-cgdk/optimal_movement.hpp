#pragma once

#include "point.hpp"
#include "context.hpp"

#include <vector>

namespace strategy {

class MovementState {
public:
    MovementState(double tick, const Point& position, double angle)
        : tick_(tick), position_(position), angle_(angle) {}

    double tick() const {
        return tick_;
    }

    const Point& position() const {
        return position_;
    }

    double angle() const {
        return angle_;
    }

private:
    double tick_;
    Point position_;
    double angle_;
};

struct Movement {
public:
    Movement(double speed, double strafe_speed, double turn)
        : speed_(speed), strafe_speed_(strafe_speed), turn_(turn) {}

    double speed() const {
        return speed_;
    }

    double strafe_speed() const {
        return strafe_speed_;
    }

    double turn() const {
        return turn_;
    }

private:
    double speed_;
    double strafe_speed_;
    double turn_;
};

using MovementsStates = std::vector<MovementState>;
using Movements = std::vector<Movement>;
using OptPoint = std::pair<bool, Point>;

std::pair<MovementsStates, Movements> get_optimal_movement(const Context& context, const strategy::Path& path, const OptPoint& look_target);

}
