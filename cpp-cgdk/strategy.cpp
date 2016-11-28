#include "strategy.hpp"
#include "optimal_destination.hpp"

#ifdef STRATEGY_DEBUG

#include "debug/output.hpp"

#include <iostream>

#endif

namespace strategy {

Strategy::Strategy(const Context& context)
        : graph_(context.game()),
          battle_mode_(std::make_shared<BattleMode>()),
          move_mode_(std::make_shared<MoveMode>(graph_)),
          mode_(move_mode_),
          destination_(get_position(context.self())),
          state_(states_.end()),
          movement_(movements_.end()) {
}

void Strategy::apply(Context &context) {
    select_mode(context);
    apply_mode(context);
    update_movements(context);
    apply_move(context);
    apply_action(context);
}

void Strategy::select_mode(const Context& context) {
    if (!context.self().getMessages().empty()) {
        return use_move_mode();
    }

    const IsInMyRange is_in_my_range {context, 0.95 * context.self().getVisionRange()};

    const auto bonuses = get_units<model::Bonus>(context.world());
    const auto has_near_bonuses = bonuses.end() != std::find_if(bonuses.begin(), bonuses.end(), is_in_my_range);

    if (has_near_bonuses) {
        return use_battle_mode();
    }

    const auto is_enemy_in_node_range = [&] (const auto& unit) {
        return is_enemy(unit, context.self().getFaction()) && is_in_my_range(unit);
    };

    const auto& buildings = get_units<model::Building>(context.world());
    const auto& minions = get_units<model::Minion>(context.world());
    const auto& wizards = get_units<model::Wizard>(context.world());

    const auto has_near_enemies = buildings.end() != std::find_if(buildings.begin(), buildings.end(), is_enemy_in_node_range)
        || minions.end() != std::find_if(minions.begin(), minions.end(), is_enemy_in_node_range)
        || wizards.end() != std::find_if(wizards.begin(), wizards.end(), is_enemy_in_node_range);

    if (has_near_enemies) {
        use_battle_mode();
    } else {
        use_move_mode();
    }
}

void Strategy::apply_mode(const Context& context) {
    const auto result = mode_->apply(context);

    if (result.active()) {
        target_ = result.target();
        if (result.destination() != destination_) {
            destination_ = result.destination();
            calculate_movements(context);
        }
    }
}

void Strategy::update_movements(const Context& context) {
    if (movement_ != movements_.end() && state_ != states_.end()) {
        const auto error = state_->position().distance(get_position(context.self())) - context.game().getWizardForwardSpeed();
        if (error <= 0) {
            ++movement_;
            ++state_;
            return;
        }
    }
    calculate_movements(context);
}

void Strategy::apply_move(Context& context) {
    if (movement_ == movements_.end()) {
        return;
    }
    context.move().setSpeed(movement_->speed());
    context.move().setStrafeSpeed(movement_->strafe_speed());
    context.move().setTurn(movement_->turn());
}

void Strategy::calculate_movements(const Context& context) {
    path_ = get_optimal_path(context, destination_, OPTIMAL_PATH_STEP_SIZE, OPTIMAL_PATH_MAX_TICKS);
    if (const auto unit = target_.circular_unit(context.cache())) {
        std::tie(states_, movements_) = get_optimal_movement(context, path_, {true, get_position(*unit)});
    } else {
        std::tie(states_, movements_) = get_optimal_movement(context, path_, {false, Point()});
    }
    state_ = states_.begin();
    movement_ = movements_.begin();
}

void Strategy::apply_action(Context& context) {
    if (const auto target = target_.circular_unit(context.cache())) {
        const auto distance = get_position(*target).distance(get_position(context.self()));

        if (distance > context.self().getCastRange() + target->getRadius() + context.game().getMagicMissileRadius()) {
            return;
        }

        const auto turn = context.self().getAngleTo(*target);

        context.move().setTurn(turn);

        if (target_.is<model::Bonus>()) {
            return;
        }

        context.move().setCastAngle(turn);

        if (context.self().getRemainingActionCooldownTicks() != 0) {
            return;
        }

        if (need_apply_staff(context, *target)) {
            context.move().setAction(model::ACTION_STAFF);
            return;
        }

        if (need_apply_magic_missile(context, *target, turn)) {
            context.move().setAction(model::ACTION_MAGIC_MISSILE);
            context.move().setMinCastDistance(distance - target->getRadius() - context.game().getMagicMissileRadius());
            return;
        }
    }
}

void Strategy::use_move_mode() {
    mode_ = move_mode_;
}

void Strategy::use_battle_mode() {
    mode_ = battle_mode_;
}

bool Strategy::need_apply_staff(const Context& context, const model::CircularUnit& target) {
    const auto distance = get_position(target).distance(get_position(context.self()));
    const auto lethal_area = target.getRadius() + context.game().getStaffRange();
    return context.self().getRemainingCooldownTicksByAction()[model::ACTION_STAFF] == 0 && distance < lethal_area;
}

bool Strategy::need_apply_magic_missile(const Context& context, const model::CircularUnit& target, double turn) {
    if (context.self().getRemainingCooldownTicksByAction()[model::ACTION_MAGIC_MISSILE] != 0) {
        return false;
    }

    const auto direction = Point(1, 0).rotated(normalize_angle(context.self().getAngle() + turn));
    const auto distance = get_position(target).distance(get_position(context.self()));
    const auto missile_target = get_position(context.self()) + direction * distance;
    const auto distance_to_target = get_position(target).distance(missile_target);
    const auto lethal_area = context.game().getMagicMissileRadius() + target.getRadius();

    if (distance_to_target > lethal_area) {
        return false;
    }

    const Circle missile(get_position(context.self()), context.game().getMagicMissileRadius());
    const auto friend_wizards = filter_friends(get_units<model::Wizard>(context.world()), context.self().getFaction(),
                                               context.self().getId());
    std::vector<Circle> barriers;
    barriers.reserve(friend_wizards.size());
    std::transform(friend_wizards.begin(), friend_wizards.end(), std::back_inserter(barriers), make_circle);

    return !has_intersection_with_barriers(missile, missile_target, barriers);
}

}
