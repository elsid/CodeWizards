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

BaseStrategy::BaseStrategy(const Context& context)
        : graph_(context.game()),
          battle_mode_(std::make_shared<BattleMode>()),
          move_mode_(std::make_shared<MoveMode>(graph_)),
          retreat_mode_(std::make_shared<RetreatMode>(battle_mode_, move_mode_)),
          mode_(move_mode_),
          destination_(get_position(context.self())),
          state_(states_.end()),
          movement_(movements_.end()) {
}

void BaseStrategy::apply(Context &context) {
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
    context.check_timeout(__PRETTY_FUNCTION__, __FILE__, __LINE__);
    if (context.self().isMaster()) {
        command(context);
    }
    context.check_timeout(__PRETTY_FUNCTION__, __FILE__, __LINE__);
}

void BaseStrategy::handle_messages(const Context& context) {
    if (!context.self().getMessages().empty()) {
        skill_from_message_ = context.self().getMessages().back().getSkillToLearn();
    }
}

void BaseStrategy::learn_skills(Context& context) {
    if (context.self().getSkills().size() < std::size_t(context.self().getLevel())) {
        const auto skill = get_skill_to_learn(context, skill_from_message_);
        if (skill != model::_SKILL_UNKNOWN_) {
            context.move().setSkillToLearn(skill);
        }
    }
}

void BaseStrategy::command(Context& context) {
    if (context.world().getTickIndex() == 0) {
        context.move().setMessages({
            model::Message(model::LANE_TOP, model::_SKILL_UNKNOWN_, {}),
            model::Message(model::LANE_MIDDLE, model::_SKILL_UNKNOWN_, {}),
            model::Message(model::LANE_MIDDLE, model::_SKILL_UNKNOWN_, {}),
            model::Message(model::LANE_BOTTOM, model::_SKILL_UNKNOWN_, {}),
        });
    }
}

void BaseStrategy::select_mode(const Context& context) {
    const auto mean_life_change = get_units<model::Wizard>(context.cache()).at(context.self().getId()).mean_life_change_speed();
    if (((mean_life_change < 0 && - context.self().getLife() / mean_life_change < TICKS_TO_DEATH_FOR_RETREAT)
         || context.self().getLife() < context.self().getMaxLife() / 3)
            && mode_ != move_mode_) {
        return use_retreat_mode();
    }

    double max_distance = get_max_distance_for_unit_candidate(context);

    if (mode_ == battle_mode_) {
        max_distance = context.game().getStaffRange() + (max_distance - context.game().getStaffRange())
                * bounded_line_factor(mode_ticks_, BATTLE_MODE_TICKS, BATTLE_MODE_TICKS - context.game().getWizardActionCooldownTicks());
    }

    if (has_candidates(context, max_distance)) {
        use_battle_mode();
    } else {
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

void BaseStrategy::apply_move(Context& context) {
    if (movement_ != movements_.end()) {
        context.move().setSpeed(movement_->speed());
        context.move().setStrafeSpeed(movement_->strafe_speed());
        context.move().setTurn(movement_->turn());
    }
}

void BaseStrategy::apply_action(Context& context) {
    const auto actions = get_actions_by_priority_order(context, target_);

    for (const auto action_type : actions) {
        bool need_apply;
        Action action;
        std::tie(need_apply, action) = need_apply_action(context, target_, action_type);

        if (need_apply) {
            context.move().setAction(action_type);
            context.move().setCastAngle(action.cast_angle);
            context.move().setMinCastDistance(action.min_cast_distance);
            context.move().setMaxCastDistance(action.max_cast_distance);
            break;
        }
    }
}

void BaseStrategy::calculate_movements(const Context& context) {
    path_ = GetOptimalPath()
            .step_size(OPTIMAL_PATH_STEP_SIZE)
            .max_ticks(OPTIMAL_PATH_MAX_TICKS)
            .max_iterations(OPTIMAL_PATH_MAX_ITERATIONS)
            (context, destination_);
    if (const auto unit = target_.circular_unit(context.cache())) {
        std::tie(states_, movements_) = get_optimal_movement(context, path_, {true, get_position(*unit) + get_speed(*unit)});
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
