#pragma once

#include "point.hpp"

namespace strategy {
namespace simulation {

class MinionMove {
public:
    void speed(double value) {
        speed_ = value;
    }

    double speed() const {
        return speed_;
    }

    void turn(double value) {
        turn_ = value;
    }

    double turn() const {
        return turn_;
    }

private:
    double speed_ = 0;
    double turn_ = 0;
};

} // namespace simulation
} // namespace strategy
