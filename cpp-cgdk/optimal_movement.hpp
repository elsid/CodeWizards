#pragma once

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

template <class Unit>
class UnitBounds {
public:
    UnitBounds(const Unit& unit, const model::Game& game)
        : game_(game),
          hastened_remaining_ticks_(get_hastened_remaining_ticks(unit)),
          movement_skill_bonus_level_(get_movement_bonus_level(unit)) {}

    double max_speed(double tick) const {
        return game_.getWizardForwardSpeed() * movement_bonus_factor(tick);
    }

    double min_speed(double tick) const {
        return -game_.getWizardBackwardSpeed() * movement_bonus_factor(tick);
    }

    double max_strafe_speed(double tick) const {
        return game_.getWizardStrafeSpeed() * movement_bonus_factor(tick);
    }

    double max_turn(double tick) const {
        return game_.getWizardMaxTurnAngle() * rotation_bonus_factor(tick);
    }

    double min_turn(double tick) const {
        return -game_.getWizardMaxTurnAngle() * rotation_bonus_factor(tick);
    }

    double movement_bonus_factor(double tick) const {
        return 1 + (tick < hastened_remaining_ticks_ ? game_.getHastenedMovementBonusFactor() : 0)
                + movement_skill_bonus_level_ * game_.getMovementBonusFactorPerSkillLevel();
    }

    double rotation_bonus_factor(double tick) const {
        return 1 + (tick < hastened_remaining_ticks_ ? game_.getHastenedRotationBonusFactor() : 0);
    }

    double limit_turn(double value, double tick) const {
        return std::min(max_turn(tick), std::max(min_turn(tick), value));
    }

private:
    const model::Game& game_;
    int hastened_remaining_ticks_;
    int movement_skill_bonus_level_;
};

template <class Unit>
UnitBounds<Unit> make_unit_bounds(const Unit& unit, const model::Game& game) {
    return UnitBounds<Unit>(unit, game);
}

using WizardBounds = UnitBounds<model::Wizard>;
using MovementsStates = std::vector<MovementState>;
using Movements = std::vector<Movement>;
using OptPoint = std::pair<bool, Point>;

std::pair<MovementsStates, Movements> get_optimal_movement(const Context& context, const Path& path, const OptPoint& look_target);

Movement get_next_movement(const Point& target, const MovementState& state, const OptPoint& look_target, const WizardBounds& bounds);
Point get_shift(const MovementState& state, const Movement& movement);
std::pair<MovementState, Movement> get_next_state(const Point& target, const MovementState& state, const OptPoint& look_target, const WizardBounds& bounds);

inline std::ostream&operator <<(std::ostream& stream, const MovementState& value) {
    return stream << std::setprecision(std::numeric_limits<double>::max_digits10)
                  << "MovementState(" << value.tick() << ", " << value.position() << ", " << value.angle() << ")";
}

inline std::ostream&operator <<(std::ostream& stream, const Movement& value) {
    return stream << std::setprecision(std::numeric_limits<double>::max_digits10)
                  << "Movement(" << value.speed() << ", " << value.strafe_speed() << ", " << value.turn() << ")";
}

}
