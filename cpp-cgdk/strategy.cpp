#include "strategy.hpp"
#include "optimal_destination.hpp"
#include "move_mode.hpp"
#include "battle_mode.hpp"

namespace strategy {

Strategy::Strategy(const Context& context)
        : graph_(context.game()),
          mode_(std::make_unique<MoveMode>(graph_)) {
    std::cout << __func__ << std::endl;
}

void Strategy::apply(Context &context) {
    update_cache(context);
    handle_messages(context);
    apply_mode(context);
    update_movements(context);
    apply_move(context);
    apply_action(context);
}

void Strategy::update_cache(const Context& context) {
    update_specific_cache<model::Bonus>(context);
    update_specific_cache<model::Building>(context);
    update_specific_cache<model::Minion>(context);
    update_specific_cache<model::Projectile>(context);
    update_specific_cache<model::Tree>(context);
    update_specific_cache<model::Wizard>(context);
}

void Strategy::handle_messages(const Context& context) {
    if (!context.self().getMessages().empty()) {
        use_move_mode();
    }
}

void Strategy::apply_mode(const Context& context) {
    IsInMyRange is_in_my_range {context, context.self().getVisionRange()};

    const auto is_enemy_in_node_range = [&] (const auto& unit) {
        return is_enemy(unit, context.self().getFaction()) && is_in_my_range(unit);
    };

    const auto& minions = get_units<model::Minion>(context.world());
    const auto& wizards = get_units<model::Wizard>(context.world());

    const auto has_near_enemies = minions.end() != std::find_if(minions.begin(), minions.end(), is_enemy_in_node_range)
        || wizards.end() != std::find_if(wizards.begin(), wizards.end(), is_enemy_in_node_range);

    if (has_near_enemies) {
        use_battle_mode();
    } else {
        use_move_mode();
    }

    const auto result = mode_->apply(context);

    if (result.active()) {
        target_ = result.target();
        if (result.destination() != destination_) {
            calculate_movements(context);
        }
    }
}

void Strategy::update_movements(const Context& context) {
    if (movement_ == movements_.end()
            || state_->position().distance(get_position(context.self())) > context.self().getRadius()) {
        calculate_movements(context);
    }
    ++movement_;
    ++state_;
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
    const auto path = get_optimal_path(context, destination_, OPTIMAL_PATH_STEP_SIZE);
    if (const auto unit = target_.unit()) {
        std::tie(states_, movements_) = get_optimal_movement(context, path, {true, get_position(*unit)});
    } else {
        std::tie(states_, movements_) = get_optimal_movement(context, path, {false, Point()});
    }
    state_ = states_.begin();
    movement_ = movements_.begin();
}

void Strategy::apply_action(Context& context) {
    if (const auto target = target_.unit()) {
        const auto distance = get_position(*target).distance(get_position(context.self()));

        if (distance > context.self().getCastRange() + target->getRadius() + context.game().getMagicMissileRadius()) {
            return;
        }

        const auto turn = context.self().getAngleTo(*target);

        context.move().setTurn(turn);

        if (target_.bonus()) {
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
    mode_ = std::make_unique<MoveMode>(graph_);
}

void Strategy::use_battle_mode() {
    mode_ = std::make_unique<BattleMode>();
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

    return has_intersection_with_barriers(missile, missile_target, barriers);
}

}
