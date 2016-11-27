#include "strategy.hpp"
#include "optimal_destination.hpp"

namespace strategy {

Strategy::Strategy(const Context& context)
        : graph_(context.game()),
          battle_mode_(std::make_shared<BattleMode>()),
          move_mode_(std::make_shared<MoveMode>(graph_)),
          mode_(move_mode_),
          destination_(get_position(context.self())) {
}

void Strategy::apply(Context &context) {
    Context context_with_cache(context.self(), context.world(), context.game(), context.move(),
                               cache_, context.profiler(), context.time_limit());
    update_cache(context_with_cache);
    select_mode(context_with_cache);
    apply_mode(context_with_cache);
    update_movements(context_with_cache);
    apply_move(context_with_cache);
    apply_action(context_with_cache);
}

template <class T>
struct CacheTtl {};

template <>
struct CacheTtl<model::Bonus> {
    static constexpr const Tick value = 2500;
};

template <>
struct CacheTtl<model::Building> {
    static constexpr const Tick value = 2500;
};

template <>
struct CacheTtl<model::Minion> {
    static constexpr const Tick value = 30;
};

template <>
struct CacheTtl<model::Projectile> {
    static constexpr const Tick value = 10;
};

template <>
struct CacheTtl<model::Tree> {
    static constexpr const Tick value = 2500;
};

template <>
struct CacheTtl<model::Wizard> {
    static constexpr const Tick value = 30;
};

void Strategy::update_cache(const Context& context) {
    strategy::update_cache(cache_, context.world());

    const auto is_friend_or_me = [&] (const auto& unit) {
        return unit.getFaction() == context.self().getFaction();
    };

    const auto buildings = filter_units(get_units<model::Building>(context.world()), is_friend_or_me);
    const auto minions = filter_units(get_units<model::Minion>(context.world()), is_friend_or_me);
    const auto wizards = filter_units(get_units<model::Wizard>(context.world()), is_friend_or_me);

    const auto need_invalidate = [&] (const auto& unit) {
        using Type = typename std::decay<decltype(unit.value())>::type;

        if (context.world().getTickIndex() - unit.last_seen() > CacheTtl<Type>::value) {
            return true;
        }

        const auto is_in_range = [&] (auto other) {
            return unit.last_seen() < context.world().getTickIndex()
                    && get_position(*other).distance(get_position(unit.value()))
                    <= other->getVisionRange() - 2 * unit.value().getRadius();
        };

        return buildings.end() != std::find_if(buildings.begin(), buildings.end(), is_in_range)
                || minions.end() != std::find_if(minions.begin(), minions.end(), is_in_range)
                || wizards.end() != std::find_if(wizards.begin(), wizards.end(), is_in_range);
    };

    invalidate_cache(cache_, need_invalidate);
}

void Strategy::select_mode(const Context& context) {
//    if (context.world().getTickIndex() % 50 != 0) {
//        return;
//    }

    if (!context.self().getMessages().empty()) {
        return use_move_mode();
    }

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
}

void Strategy::apply_mode(const Context& context) {
//    if (context.world().getTickIndex() % 50 != 0) {
//        return;
//    }

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
    const auto error = state_->position().distance(get_position(context.self())) - context.self().getRadius();
    if (movement_ == movements_.end() || error > 0) {
        return calculate_movements(context);
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
    path_ = get_optimal_path(context, destination_, OPTIMAL_PATH_STEP_SIZE, OPTIMAL_PATH_MAX_TICKS, context.time_left());
    if (const auto unit = target_.circular_unit(cache_)) {
        std::tie(states_, movements_) = get_optimal_movement(context, path_, {true, get_position(*unit)});
    } else {
        std::tie(states_, movements_) = get_optimal_movement(context, path_, {false, Point()});
    }
    state_ = states_.begin();
    movement_ = movements_.begin();
}

void Strategy::apply_action(Context& context) {
    if (const auto target = target_.circular_unit(cache_)) {
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
