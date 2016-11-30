#include "base_strategy.hpp"
#include "optimal_destination.hpp"
#include "optimal_position.hpp"

#ifdef STRATEGY_DEBUG

#include "debug/output.hpp"

#include <iostream>

#endif

namespace strategy {

static const std::vector<model::SkillType> SKILLS = {
    model::SKILL_SHIELD,
    model::SKILL_HASTE,
    model::SKILL_ADVANCED_MAGIC_MISSILE,
    model::SKILL_FROST_BOLT,
    model::SKILL_FIREBALL,
    model::SKILL_RANGE_BONUS_PASSIVE_1,
    model::SKILL_RANGE_BONUS_AURA_1,
    model::SKILL_RANGE_BONUS_PASSIVE_2,
    model::SKILL_RANGE_BONUS_AURA_2,
    model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_1,
    model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_1,
    model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_2,
    model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_2,
    model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_1,
    model::SKILL_STAFF_DAMAGE_BONUS_AURA_1,
    model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_2,
    model::SKILL_STAFF_DAMAGE_BONUS_AURA_2,
    model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_1,
    model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_1,
    model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_2,
    model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_2,
    model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_1,
    model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_1,
    model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_2,
    model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_2,
};

bool has_skill(const model::Wizard& unit, model::SkillType skill) {
    return unit.getSkills().end() != std::find(unit.getSkills().begin(), unit.getSkills().end(), skill);
}

BaseStrategy::BaseStrategy(const Context& context)
        : graph_(context.game()),
          battle_mode_(std::make_shared<BattleMode>()),
          move_mode_(std::make_shared<MoveMode>(graph_)),
          mode_(move_mode_),
          destination_(get_position(context.self())),
          state_(states_.end()),
          movement_(movements_.end()) {
}

void BaseStrategy::apply(Context &context) {
    context.check_timeout(__PRETTY_FUNCTION__, __FILE__, __LINE__);
    select_mode(context);
    context.check_timeout(__PRETTY_FUNCTION__, __FILE__, __LINE__);
    apply_mode(context);
    context.check_timeout(__PRETTY_FUNCTION__, __FILE__, __LINE__);
    update_movements(context);
    context.check_timeout(__PRETTY_FUNCTION__, __FILE__, __LINE__);
    apply_move(context);
    context.check_timeout(__PRETTY_FUNCTION__, __FILE__, __LINE__);
    apply_action(context);
    context.check_timeout(__PRETTY_FUNCTION__, __FILE__, __LINE__);
    learn_skills(context);
    context.check_timeout(__PRETTY_FUNCTION__, __FILE__, __LINE__);
}

void BaseStrategy::learn_skills(Context& context) {
    if (!context.self().getMessages().empty()) {
        last_message_ = {true, context.self().getMessages().back()};
    }
    if (last_message_.first && !has_skill(context.self(), last_message_.second.getSkillToLearn())) {
        context.move().setSkillToLearn(last_message_.second.getSkillToLearn());
    } else {
        std::vector<std::size_t> skills_frequency(model::_SKILL_COUNT_);
        for (const auto& wizard : get_units<model::Wizard>(context.cache())) {
            for (const auto skill : wizard.second.value().getSkills()) {
                ++skills_frequency[skill];
            }
        }
        const auto frequent = std::max_element(skills_frequency.begin(), skills_frequency.end());
        const auto frequent_value = *frequent;
        std::sort(skills_frequency.begin(), skills_frequency.end());
        model::SkillType skill;
        if (frequent_value <= skills_frequency[skills_frequency.size() / 2]) {
            skill = *std::find_if(SKILLS.begin(), SKILLS.end(), [&] (auto skill) { return !has_skill(context.self(), skill); });
        } else {
            skill = model::SkillType(frequent - skills_frequency.begin());
        }
        context.move().setSkillToLearn(skill);
    }
}

void BaseStrategy::select_mode(const Context& context) {
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
    if (movement_ == movements_.end()) {
        return;
    }

    context.move().setSpeed(movement_->speed());
    context.move().setStrafeSpeed(movement_->strafe_speed());

    if (const auto target = target_.circular_unit(context.cache())) {
        const Bounds bounds(context);
        const auto turn = bounds.limit_turn(context.self().getAngleTo(*target), 0);

        context.move().setTurn(turn);

        if (target_.is<model::Bonus>()) {
            return;
        }

        context.move().setCastAngle(turn);
    } else {
        context.move().setTurn(movement_->turn());
    }
}

void BaseStrategy::calculate_movements(const Context& context) {
    path_ = get_optimal_path(context, destination_, OPTIMAL_PATH_STEP_SIZE, OPTIMAL_PATH_MAX_TICKS);
    if (const auto unit = target_.circular_unit(context.cache())) {
        std::tie(states_, movements_) = get_optimal_movement(context, path_, {true, get_position(*unit)});
    } else {
        std::tie(states_, movements_) = get_optimal_movement(context, path_, {false, Point()});
    }
    state_ = states_.begin();
    movement_ = movements_.begin();
}

void BaseStrategy::apply_action(Context& context) {
    if (context.self().getRemainingActionCooldownTicks() > 0) {
        return;
    } else if (need_apply_haste(context)) {
        context.move().setAction(model::ACTION_HASTE);
        context.move().setStatusTargetId(context.self().getId());
    } else if (need_apply_shield(context)) {
        context.move().setAction(model::ACTION_SHIELD);
        context.move().setStatusTargetId(context.self().getId());
    } else if (target_.is<model::Bonus>()) {
        return;
    } else if (const auto target = target_.circular_unit(context.cache())) {
        const auto distance = get_position(context.self()).distance(get_position(*target));
        if (need_apply_staff(context, *target)) {
            context.move().setAction(model::ACTION_STAFF);
        } else if (need_apply_fireball(context, *target)) {
            context.move().setAction(model::ACTION_FIREBALL);
            context.move().setMinCastDistance(distance - target->getRadius() - context.game().getFireballRadius());
        } else if (need_apply_frostbolt(context, *target)) {
            context.move().setAction(model::ACTION_FROST_BOLT);
            context.move().setMinCastDistance(distance - target->getRadius() - context.game().getFrostBoltRadius());
        } else if (need_apply_magic_missile(context, *target)) {
            context.move().setAction(model::ACTION_MAGIC_MISSILE);
            context.move().setMinCastDistance(distance - target->getRadius() - context.game().getMagicMissileRadius());
        }
    }
}

void BaseStrategy::use_move_mode() {
    mode_ = move_mode_;
}

void BaseStrategy::use_battle_mode() {
    mode_ = battle_mode_;
}

bool BaseStrategy::need_apply_haste(const Context& context) const {
    return context.self().getRemainingCooldownTicksByAction()[model::ACTION_HASTE] == 0
            && context.self().getMana() >= context.game().getHasteManacost()
            && !is_with_status(context.self(), model::STATUS_HASTENED)
            && has_skill(context.self(), model::SKILL_HASTE)
            && ((!target_.is<model::Bonus>()
                && target_.is_some())
                || get_position(context.self()).distance(destination_) > 0.5 * context.self().getVisionRange());
}

bool BaseStrategy::need_apply_shield(const Context& context) const {
    return context.self().getRemainingCooldownTicksByAction()[model::ACTION_SHIELD] == 0
            && context.self().getMana() >= context.game().getShieldManacost()
            && !is_with_status(context.self(), model::STATUS_SHIELDED)
            && !target_.is<model::Bonus>()
            && target_.is_some()
            && has_skill(context.self(), model::SKILL_SHIELD);
}

bool BaseStrategy::need_apply_staff(const Context& context, const model::CircularUnit& target) {
    const auto distance = get_position(target).distance(get_position(context.self()));
    const auto lethal_area = target.getRadius() + context.game().getStaffRange();
    return context.self().getRemainingCooldownTicksByAction()[model::ACTION_STAFF] == 0
            && distance < lethal_area;
}

bool BaseStrategy::need_apply_fireball(const Context& context, const model::CircularUnit& target) {
    return context.self().getRemainingCooldownTicksByAction()[model::ACTION_FIREBALL] == 0
            && context.self().getMana() >= context.game().getFireballManacost()
            && has_skill(context.self(), model::SKILL_FIREBALL)
            && need_apply_cast(context, target, context.game().getFireballRadius());
}

bool BaseStrategy::need_apply_frostbolt(const Context& context, const model::CircularUnit& target) {
    return context.self().getRemainingCooldownTicksByAction()[model::ACTION_FROST_BOLT] == 0
            && context.self().getMana() >= context.game().getFrostBoltManacost()
            && has_skill(context.self(), model::SKILL_FROST_BOLT)
            && need_apply_cast(context, target, context.game().getFrostBoltRadius());
}

bool BaseStrategy::need_apply_magic_missile(const Context& context, const model::CircularUnit& target) {
    return context.self().getRemainingCooldownTicksByAction()[model::ACTION_MAGIC_MISSILE] == 0
            && context.self().getMana() >= context.game().getMagicMissileManacost()
            && need_apply_cast(context, target, context.game().getMagicMissileRadius());
}

bool BaseStrategy::need_apply_cast(const Context& context, const model::CircularUnit& target, double radius) {
    const auto direction = Point(1, 0).rotated(normalize_angle(context.self().getAngle() + context.move().getCastAngle()));
    const auto distance = get_position(target).distance(get_position(context.self()));
    const auto cast_target = get_position(context.self()) + direction * std::min(distance, context.self().getCastRange());
    const auto distance_to_target = get_position(target).distance(cast_target);
    const auto lethal_area = radius + target.getRadius() * 0.5;

    if (distance_to_target > lethal_area) {
        return false;
    }

    const Circle cast(get_position(context.self()), radius);
    const auto friend_wizards = filter_friends(get_units<model::Wizard>(context.world()), context.self().getFaction(),
                                               context.self().getId());
    std::vector<Circle> barriers;
    barriers.reserve(friend_wizards.size());
    std::transform(friend_wizards.begin(), friend_wizards.end(), std::back_inserter(barriers), make_circle);

    return !has_intersection_with_barriers(cast, cast_target, barriers);
}

}
