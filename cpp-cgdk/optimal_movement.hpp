#ifndef STRATEGY_OPTIMAL_MOVEMENT_HPP
#define STRATEGY_OPTIMAL_MOVEMENT_HPP

#include "point.hpp"
#include "context.hpp"
#include "optimal_path.hpp"
#include "helpers.hpp"

#include <vector>

namespace strategy {

class MovementState {
public:
    MovementState(int tick, const Point& position, double angle)
        : tick_(tick), position_(position), angle_(angle) {}

    int tick() const {
        return tick_;
    }

    const Point& position() const {
        return position_;
    }

    double angle() const {
        return angle_;
    }

private:
    int tick_;
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

class Bounds {
public:
    Bounds(const Context& context)
        : context_(context), hastened_remaining_ticks_(get_hastened_remaining_ticks(context.self())) {}

    double max_speed(double tick) const {
        return context_.game().getWizardForwardSpeed() * movement_bonus_factor(tick);
    }

    double min_speed(double tick) const {
        return -context_.game().getWizardBackwardSpeed() * movement_bonus_factor(tick);
    }

    double max_strafe_speed(double tick) const {
        return context_.game().getWizardStrafeSpeed() * movement_bonus_factor(tick);
    }

    double max_turn(double tick) const {
        return context_.game().getWizardMaxTurnAngle() * rotation_bonus_factor(tick);
    }

    double min_turn(double tick) const {
        return -context_.game().getWizardMaxTurnAngle() * rotation_bonus_factor(tick);
    }

    double movement_bonus_factor(double tick) const {
        return 1 + (tick < hastened_remaining_ticks_ ? context_.game().getHastenedMovementBonusFactor() : 0);
    }

    double rotation_bonus_factor(double tick) const {
        return 1 + (tick < hastened_remaining_ticks_ ? context_.game().getHastenedRotationBonusFactor() : 0);
    }

    double limit_turn(double value, double tick) const {
        return std::min(max_turn(tick), std::max(min_turn(tick), value));
    }

private:
    const Context& context_;
    int hastened_remaining_ticks_;
};

using MovementsStates = std::vector<MovementState>;
using Movements = std::vector<Movement>;
using OptPoint = std::pair<bool, Point>;

std::pair<MovementsStates, Movements> get_optimal_movement(const Context& context, const Path& path, const OptPoint& look_target);

Movement get_next_movement(const Point& target, const MovementState& state, const OptPoint& look_target, const Bounds& bounds);
Point get_shift(const MovementState& state, const Movement& movement);
std::pair<MovementState, Movement> get_next_state(const Point& target, const MovementState& state, const OptPoint& look_target, const Bounds& bounds);

inline std::ostream&operator <<(std::ostream& stream, const MovementState& value) {
    return stream << std::setprecision(std::numeric_limits<double>::max_digits10)
                  << "MovementState(" << value.tick() << ", " << value.position() << ", " << value.angle() << ")";
}

inline std::ostream&operator <<(std::ostream& stream, const Movement& value) {
    return stream << std::setprecision(std::numeric_limits<double>::max_digits10)
                  << "Movement(" << value.speed() << ", " << value.strafe_speed() << ", " << value.turn() << ")";
}

}

#endif
