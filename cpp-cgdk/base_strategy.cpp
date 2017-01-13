#include "base_strategy.hpp"
#include "optimal_destination.hpp"
#include "optimal_position.hpp"
#include "action.hpp"
#include "skills.hpp"

#ifdef ELSID_STRATEGY_DEBUG

#include "debug/output.hpp"

#include <iostream>

#endif

namespace strategy {

std::ostream& operator <<(std::ostream& stream, model::ActionType value) {
    switch (value) {
        case model::_ACTION_UNKNOWN_:
            return stream << "model::_ACTION_UNKNOWN_";
        case model::ACTION_NONE:
            return stream << "model::ACTION_NONE";
        case model::ACTION_STAFF:
            return stream << "model::ACTION_STAFF";
        case model::ACTION_MAGIC_MISSILE:
            return stream << "model::ACTION_MAGIC_MISSILE";
        case model::ACTION_FROST_BOLT:
            return stream << "model::ACTION_FROST_BOLT";
        case model::ACTION_FIREBALL:
            return stream << "model::ACTION_FIREBALL";
        case model::ACTION_HASTE:
            return stream << "model::ACTION_HASTE";
        case model::ACTION_SHIELD:
            return stream << "model::ACTION_SHIELD";
        case model::_ACTION_COUNT_:
            return stream << "model::_ACTION_COUNT_";
    }
    return stream;
}

BaseStrategy::BaseStrategy(const Context& context)
        : graph_(context.game()),
          battle_mode_(std::make_shared<BattleMode>()),
          move_mode_(std::make_shared<MoveMode>(graph_)),
          retreat_mode_(std::make_shared<RetreatMode>(battle_mode_, move_mode_)),
          destination_(get_position(context.self())),
          state_(states_.end()),
          movement_(movements_.end()),
          stats_(*this) {
}

void BaseStrategy::apply(Context &context) {
    stats_.calculate(context);
    context.check_timeout(__PRETTY_FUNCTION__, __FILE__, __LINE__);
    if (!context.self().isMaster()) {
        handle_messages(context);
    }
    context.check_timeout(__PRETTY_FUNCTION__, __FILE__, __LINE__);
    select_mode(context);
    context.check_timeout(__PRETTY_FUNCTION__, __FILE__, __LINE__);
    apply_mode(context);
    context.check_timeout(__PRETTY_FUNCTION__, __FILE__, __LINE__);
    apply_move(context);
    context.check_timeout(__PRETTY_FUNCTION__, __FILE__, __LINE__);
    apply_action(context);
    context.check_timeout(__PRETTY_FUNCTION__, __FILE__, __LINE__);
    learn_skills(context);
    context.check_timeout(__PRETTY_FUNCTION__, __FILE__, __LINE__);
    update_movements(context);
}

void BaseStrategy::handle_messages(const Context& context) {
    if (!context.self().getMessages().empty()) {
        skill_from_message_ = context.self().getMessages().back().getSkillToLearn();
    }
}

void BaseStrategy::learn_skills(Context& context) const {
    if (context.self().getSkills().size() < std::size_t(context.self().getLevel())) {
        const auto skill = get_skill_to_learn(context, skill_from_message_);
        if (skill != model::_SKILL_UNKNOWN_) {
            context.move().setSkillToLearn(skill);
        }
    }
}

void BaseStrategy::select_mode(const Context& context) {
    const auto mean_life_change = get_units<model::Wizard>(context.cache()).at(context.self().getId()).mean_life_change_speed();

    if (context.self().getLife() < context.self().getMaxLife() / 3 && mode_ != move_mode_) {
        if (mode_ != retreat_mode_ || mode_ticks_ % 100 == 0) {
            SLOG(context) << "use_retreat_mode"
                << " reason: life < max_life / 3, where"
                << " life=" << context.self().getLife() << ','
                << " max_life=" << context.self().getMaxLife() << ','
                << " max_life / 3=" << context.self().getMaxLife() / 3
                << "\n";
        }
        return use_retreat_mode();
    }

    if (mean_life_change < 0 && - context.self().getLife() / mean_life_change < TICKS_TO_DEATH_FOR_RETREAT && mode_ != move_mode_) {
        if (mode_ != retreat_mode_ || mode_ticks_ % 100 == 0) {
            SLOG(context) << "use_retreat_mode"
                << " reason: mean_life_change < 0 and - life / mean_life_change < TICKS_TO_DEATH_FOR_RETREAT, where"
                << " mean_life_change=" << mean_life_change << ','
                << " life=" << context.self().getLife() << ','
                << " life / mean_life_change=" << context.self().getLife() / mean_life_change << ','
                << " TICKS_TO_DEATH_FOR_RETREAT=" << TICKS_TO_DEATH_FOR_RETREAT
                << "\n";
        }
        return use_retreat_mode();
    }

    double max_distance = get_max_distance_for_unit_candidate(context);

    if (mode_ == battle_mode_) {
        max_distance = context.game().getStaffRange() + (max_distance - context.game().getStaffRange())
                * bounded_line_factor(mode_ticks_, BATTLE_MODE_TICKS, BATTLE_MODE_TICKS - context.game().getWizardActionCooldownTicks());
    }

    if (context.world().getTickIndex() - context.cached_self().last_activity() > INACTIVE_TIMEOUT) {
        max_distance /= (context.cached_self().last_activity() - INACTIVE_TIMEOUT);
    }

    if (has_candidates(context, max_distance)) {
        if (mode_ != battle_mode_) {
            SLOG(context) << "use_battle_mode"
                << " reason: has_candidates in max_distance, where"
                << " max_distance=" << max_distance
                << "\n";
        }
        use_battle_mode();
    } else {
        if (mode_ != move_mode_) {
            SLOG(context) << "use_move_mode"
                << " reason: not has_candidates in max_distance, where"
                << " max_distance=" << max_distance
                << "\n";
        }
        use_move_mode();
    }
}

void BaseStrategy::apply_mode(const Context& context) {
    const auto result = mode_->apply(context);

    if (result.active()) {
        target_ = result.target();
        if (result.destination() != destination_) {
            destination_ = result.destination();
            calculate_movements(context);
        }
    }
}

void BaseStrategy::update_movements(const Context& context) {
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

void BaseStrategy::apply_move(Context& context) const {
    if (movement_ != movements_.end()) {
        context.move().setSpeed(movement_->speed());
        context.move().setStrafeSpeed(movement_->strafe_speed());
        context.move().setTurn(movement_->turn());
    }
}

void BaseStrategy::apply_action(Context& context) const {
    if (apply_action(context, target_)) {
        SLOG(context) << "apply_action_to_target"
            << " action_type=" << context.move().getAction()
            << " cast_angle=" << context.move().getCastAngle()
            << " min_cast_distance=" << context.move().getMinCastDistance()
            << " max_cast_distance=" << context.move().getMaxCastDistance()
            << " status_target_id=" << context.move().getStatusTargetId()
            << '\n';
        return;
    }

    if (target_.is_some()) {
        return;
    }

    std::vector<Target> candidates;
    candidates.reserve(context.world().getWizards().size() + context.world().getMinions().size() + context.world().getBuildings().size());

    const auto add_candidates = [&] (const auto& units) {
        for (const auto& unit : units) {
            if (is_enemy(unit, context.self().getFaction())) {
                candidates.push_back(Target(get_id(unit)));
            }
        }
    };

    add_candidates(context.world().getWizards());
    add_candidates(context.world().getMinions());
    add_candidates(context.world().getBuildings());

    const auto get_target_position = [&] (auto unit) { return get_position(*unit); };

    std::sort(candidates.begin(), candidates.end(),
        [&] (const auto& lhs, const auto& rhs) {
            return lhs.apply(context.cache(), get_target_position).distance(get_position(context.self()))
                    < rhs.apply(context.cache(), get_target_position).distance(get_position(context.self()));
    });

    for (const auto& candidate : candidates) {
        if (apply_action(context, candidate)) {
            SLOG(context) << "apply_action_to_candidate"
                << " action_type=" << context.move().getAction()
                << " cast_angle=" << context.move().getCastAngle()
                << " min_cast_distance=" << context.move().getMinCastDistance()
                << " max_cast_distance=" << context.move().getMaxCastDistance()
                << " status_target_id=" << context.move().getStatusTargetId()
                << '\n';
            return;
        }
    }
}

bool BaseStrategy::apply_action(Context& context, const Target& target) const {
    const auto actions = get_actions_by_priority_order(context, target);

    for (const auto action_type : actions) {
        bool need_apply;
        Action action;
        std::tie(need_apply, action) = need_apply_action(context, target, action_type);

        if (need_apply) {
            context.move().setAction(action_type);
            context.move().setCastAngle(action.cast_angle());
            context.move().setMinCastDistance(action.min_cast_distance());
            context.move().setMaxCastDistance(action.max_cast_distance());
            context.move().setStatusTargetId(action.status_target_id());
            return true;
        }
    }

    return false;
}

void BaseStrategy::calculate_movements(const Context& context) {
    ticks_states_.clear();
    steps_states_.clear();
    path_ = GetOptimalPath()
            .step_size(OPTIMAL_PATH_STEP_SIZE)
            .max_ticks(OPTIMAL_PATH_MAX_TICKS)
            .max_iterations(OPTIMAL_PATH_MAX_ITERATIONS)
#ifdef ELSID_STRATEGY_DEBUG
            .ticks_states(&ticks_states_)
            .steps_states(&steps_states_)
#endif
            (context, destination_);
    if (const auto unit = target_.circular_unit(context.cache())) {
        std::tie(states_, movements_) = get_optimal_movement(context, path_, {true, get_optimal_target_position(*unit)});
    } else {
        std::tie(states_, movements_) = get_optimal_movement(context, path_, {false, Point()});
    }
    state_ = states_.begin();
    movement_ = movements_.begin();
}

void BaseStrategy::use_move_mode() {
    use_mode(move_mode_);
}

void BaseStrategy::use_battle_mode() {
    use_mode(battle_mode_);
}

void BaseStrategy::use_retreat_mode() {
    use_mode(retreat_mode_);
}

void BaseStrategy::use_mode(const std::shared_ptr<Mode>& mode) {
    if (mode_ != mode) {
        mode_ticks_ = 0;
        mode_ = mode;
        mode_->reset();
    } else {
        ++mode_ticks_;
    }
}

}
