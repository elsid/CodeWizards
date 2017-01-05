#include "base_strategy.hpp"
#include "optimal_destination.hpp"
#include "optimal_position.hpp"
#include "action.hpp"

#ifdef ELSID_STRATEGY_DEBUG

#include "debug/output.hpp"

#include <iostream>

#endif

namespace strategy {

const std::unordered_map<model::SkillType, std::vector<model::SkillType>> SKILLS_OPPOSITE({
    {model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_1, {model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_1}},
    {model::SKILL_STAFF_DAMAGE_BONUS_AURA_1, {model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_1}},
    {model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_2, {model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_2}},
    {model::SKILL_STAFF_DAMAGE_BONUS_AURA_2, {model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_2}},
    {model::SKILL_FIREBALL, {model::SKILL_FROST_BOLT}},
    {model::SKILL_RANGE_BONUS_PASSIVE_1, {model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_1}},
    {model::SKILL_RANGE_BONUS_AURA_1, {model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_1}},
    {model::SKILL_RANGE_BONUS_PASSIVE_2, {model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_2}},
    {model::SKILL_RANGE_BONUS_AURA_2, {model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_2}},
    {model::SKILL_ADVANCED_MAGIC_MISSILE, {model::SKILL_SHIELD}},
    {model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_1, {model::SKILL_RANGE_BONUS_PASSIVE_1}},
    {model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_1, {model::SKILL_RANGE_BONUS_AURA_1}},
    {model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_2, {model::SKILL_RANGE_BONUS_PASSIVE_2}},
    {model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_2, {model::SKILL_RANGE_BONUS_AURA_2}},
    {model::SKILL_FROST_BOLT, {model::SKILL_ADVANCED_MAGIC_MISSILE}},
    {model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_1, {model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_1}},
    {model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_1, {model::SKILL_STAFF_DAMAGE_BONUS_AURA_1}},
    {model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_2, {model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_2}},
    {model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_2, {model::SKILL_STAFF_DAMAGE_BONUS_AURA_2}},
    {model::SKILL_SHIELD, {model::SKILL_FIREBALL}},
    {model::SKILL_HASTE, {model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_1}},
    {model::SKILL_HASTE, {model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_1}},
    {model::SKILL_HASTE, {model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_2}},
    {model::SKILL_HASTE, {model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_2}},
    {model::SKILL_HASTE, {model::SKILL_HASTE}},
});

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
        std::array<int, model::_SKILL_COUNT_> skills_priorities = {{
            4, // SKILL_RANGE_BONUS_PASSIVE_1
            4, // SKILL_RANGE_BONUS_AURA_1
            4, // SKILL_RANGE_BONUS_PASSIVE_2
            4, // SKILL_RANGE_BONUS_AURA_2
            4, // SKILL_ADVANCED_MAGIC_MISSILE
            5, // SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_1
            5, // SKILL_MAGICAL_DAMAGE_BONUS_AURA_1
            5, // SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_2
            5, // SKILL_MAGICAL_DAMAGE_BONUS_AURA_2
            5, // SKILL_FROST_BOLT
            3, // SKILL_STAFF_DAMAGE_BONUS_PASSIVE_1
            3, // SKILL_STAFF_DAMAGE_BONUS_AURA_1
            3, // SKILL_STAFF_DAMAGE_BONUS_PASSIVE_2
            3, // SKILL_STAFF_DAMAGE_BONUS_AURA_2
            3, // SKILL_FIREBALL
            1, // SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_1
            1, // SKILL_MOVEMENT_BONUS_FACTOR_AURA_1
            1, // SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_2
            1, // SKILL_MOVEMENT_BONUS_FACTOR_AURA_2
            1, // SKILL_HASTE
            2, // SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_1
            2, // SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_1
            2, // SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_2
            2, // SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_2
            2, // SKILL_SHIELD
        }};
        for (const auto skill : context.self().getSkills()) {
            skills_priorities[skill] = 0;
        }
        if (skill_from_message_ != model::_SKILL_UNKNOWN_ && skill_from_message_ != model::_SKILL_COUNT_
                && !has_skill(context.self(), skill_from_message_)) {
            skills_priorities[skill_from_message_] += 5;
        }
        model::SkillType opposite;
        int opposite_priority;
        std::tie(opposite, opposite_priority) = get_opposite_skill(context);
        if (opposite != model::_SKILL_UNKNOWN_) {
            skills_priorities[opposite] += opposite_priority;
        }
        const auto max = std::max_element(skills_priorities.begin(), skills_priorities.end());
        const auto skill = next_to_learn(context.self(), model::SkillType(max - skills_priorities.begin()));
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

std::pair<model::SkillType, int> BaseStrategy::get_opposite_skill(const Context& context) const {
    const auto& wizards = get_units<model::Wizard>(context.history_cache());
    auto enemy_wizards = filter_units<model::Wizard>(wizards,
        [&] (const auto& unit) { return unit.getFaction() != context.self().getFaction(); });
    std::sort(enemy_wizards.begin(), enemy_wizards.end(),
        [] (auto lhs, auto rhs) { return lhs->getXp() > rhs->getXp(); });
    for (const auto& unit : enemy_wizards) {
        const auto skills_diff = unit->getSkills().size() - context.self().getSkills().size();
        const auto distance = get_position(context.self()).distance(get_position(*unit));
        if (skills_diff > 0 && distance < 1.5 * context.self().getVisionRange()) {
            for (const auto skill : unit->getSkills()) {
                const auto opposite = SKILLS_OPPOSITE.find(skill);
                if (opposite != SKILLS_OPPOSITE.end()) {
                    for (const auto top : opposite->second) {
                        if (!has_skill(context.self(), top)) {
                            return {top, 5 * skills_diff};
                        }
                    }
                }
            }
        }
    }
    return {model::_SKILL_UNKNOWN_, 0};
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
