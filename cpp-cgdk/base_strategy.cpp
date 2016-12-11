#include "base_strategy.hpp"
#include "optimal_destination.hpp"
#include "optimal_position.hpp"

#ifdef ELSID_STRATEGY_DEBUG

#include "debug/output.hpp"

#include <iostream>

#endif

namespace strategy {

const std::unordered_map<model::SkillType, std::vector<model::SkillType>> SKILLS_OPPOSITE({
    {model::SKILL_FIREBALL, {model::SKILL_FROST_BOLT, model::SKILL_HASTE}},
    {model::SKILL_ADVANCED_MAGIC_MISSILE, {model::SKILL_FIREBALL, model::SKILL_SHIELD}},
    {model::SKILL_FROST_BOLT, {model::SKILL_ADVANCED_MAGIC_MISSILE, model::SKILL_HASTE}},
    {model::SKILL_SHIELD, {model::SKILL_FIREBALL, model::SKILL_FROST_BOLT}},
    {model::SKILL_HASTE, {model::SKILL_HASTE, model::SKILL_FROST_BOLT}},
});

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
    handle_messages(context);
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
}

void BaseStrategy::handle_messages(const Context& context) {
    if (!context.self().getMessages().empty()) {
        skill_from_message_ = context.self().getMessages().back().getSkillToLearn();
    }
}

void BaseStrategy::learn_skills(Context& context) {
    for (; prev_level_ < context.self().getLevel(); ++prev_level_) {
        std::array<int, model::_SKILL_COUNT_> skills_priorities = {{
            4, // SKILL_RANGE_BONUS_PASSIVE_1
            4, // SKILL_RANGE_BONUS_AURA_1
            4, // SKILL_RANGE_BONUS_PASSIVE_2
            4, // SKILL_RANGE_BONUS_AURA_2
            4, // SKILL_ADVANCED_MAGIC_MISSILE
            3, // SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_1
            3, // SKILL_MAGICAL_DAMAGE_BONUS_AURA_1
            3, // SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_2
            3, // SKILL_MAGICAL_DAMAGE_BONUS_AURA_2
            3, // SKILL_FROST_BOLT
            5, // SKILL_STAFF_DAMAGE_BONUS_PASSIVE_1
            5, // SKILL_STAFF_DAMAGE_BONUS_AURA_1
            5, // SKILL_STAFF_DAMAGE_BONUS_PASSIVE_2
            5, // SKILL_STAFF_DAMAGE_BONUS_AURA_2
            5, // SKILL_FIREBALL
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
            return;
        }
    }
}

std::pair<model::SkillType, int> BaseStrategy::get_opposite_skill(const Context& context) const {
    const auto& wizards = get_units<model::Wizard>(context.history_cache());
    auto enemy_wizards = filter_units<model::Wizard>(wizards,
        [&] (const auto& unit) { return unit.getFaction() != context.self().getFaction(); });
    std::sort(enemy_wizards.begin(), enemy_wizards.end(),
        [] (auto lhs, auto rhs) { return lhs->getXp() > rhs->getXp(); });
    for (const auto& unit : enemy_wizards) {
        const auto level_diff = unit->getLevel() > context.self().getLevel();
        const auto distance = get_position(context.self()).distance(get_position(*unit));
        if (level_diff > 0 && distance < 1.5 * context.self().getVisionRange()) {
            for (const auto skill : unit->getSkills()) {
                const auto opposite = SKILLS_OPPOSITE.find(skill);
                if (opposite != SKILLS_OPPOSITE.end()) {
                    for (const auto top : opposite->second) {
                        if (!has_skill(context.self(), top)) {
                            return {top, 2 * level_diff};
                        }
                    }
                }
            }
        }
    }
    return {model::_SKILL_UNKNOWN_, 0};
}

void BaseStrategy::select_mode(const Context& context) {
    const auto max_distance = context.self().getLife() < context.self().getMaxLife() / 3
            ? context.game().getStaffRange() : 1.3 * context.self().getVisionRange();
    const IsInMyRange is_in_vision_range {context, max_distance};

    const auto bonuses = get_units<model::Bonus>(context.world());
    const auto has_near_bonuses = bonuses.end() != std::find_if(bonuses.begin(), bonuses.end(), is_in_vision_range);

    if (has_near_bonuses) {
        return use_battle_mode();
    }

    const auto is_enemy_in_node_range = [&] (const auto& unit) {
        return is_enemy(unit, context.self().getFaction()) && is_in_vision_range(unit);
    };

    const auto& buildings = get_units<model::Building>(context.world());
    const auto& minions = get_units<model::Minion>(context.world());
    const auto& wizards = get_units<model::Wizard>(context.world());
    const auto& trees = get_units<model::Tree>(context.world());

    const auto is_in_staff_range = [&] (const auto& unit) {
        return get_position(context.self()).distance(get_position(unit)) <= unit.getRadius() + context.game().getStaffRange();
    };

    const auto is_neutral_in_staff_range = [&] (const auto& unit) {
        return unit.getFaction() == model::FACTION_NEUTRAL && is_in_staff_range(unit);
    };

    const auto has_near_potential_targets =
            buildings.end() != std::find_if(buildings.begin(), buildings.end(), is_enemy_in_node_range)
        || minions.end() != std::find_if(minions.begin(), minions.end(), is_enemy_in_node_range)
        || wizards.end() != std::find_if(wizards.begin(), wizards.end(), is_enemy_in_node_range)
        || trees.end() != std::find_if(trees.begin(), trees.end(), is_in_staff_range)
        || minions.end() != std::find_if(minions.begin(), minions.end(), is_neutral_in_staff_range);

    if (has_near_potential_targets) {
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
    }

    if (const auto target = target_.circular_unit(context.cache())) {
        const Bounds bounds(context);
        const auto turn = bounds.limit_turn(context.self().getAngleTo(*target), 0);
//        const auto direction = get_position(*target) + get_speed(*target) - get_position(context.self());
//        const auto turn = bounds.limit_turn(normalize_angle(direction.absolute_rotation() - context.self().getAngle()), 0);

        context.move().setTurn(turn);

        if (target_.is<model::Bonus>()) {
            return;
        }

        context.move().setCastAngle(turn);
    } else if (movement_ != movements_.end()) {
        context.move().setTurn(movement_->turn());
    }
}

void BaseStrategy::calculate_movements(const Context& context) {
    path_ = get_optimal_path(context, destination_, OPTIMAL_PATH_STEP_SIZE, OPTIMAL_PATH_MAX_TICKS, OPTIMAL_PATH_MAX_ITERATIONS);
    if (const auto unit = target_.circular_unit(context.cache())) {
        std::tie(states_, movements_) = get_optimal_movement(context, path_, {true, get_position(*unit) + get_speed(*unit)});
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
    if (mode_ != move_mode_) {
        mode_ = move_mode_;
        move_mode_->reset();
    }
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
    if (context.self().getRemainingCooldownTicksByAction()[model::ACTION_STAFF] != 0) {
        return false;
    }

    const auto is_in_attack_range = [&] (const auto& unit) {
        const auto direction = get_position(unit) - get_position(context.self());
        const auto angle = normalize_angle(direction.absolute_rotation() - context.self().getAngle());

        if (std::abs(angle) > context.game().getStaffSector()) {
            return false;
        }

        const auto distance = direction.norm();
        const auto lethal_area = unit.getRadius() + context.game().getStaffRange();
        return distance <= lethal_area;
    };

    if (!is_in_attack_range(target)) {
        return false;
    }

    const auto wizards = get_units<model::Wizard>(context.world());
    const auto is_any_friend_in_attack_range = wizards.end() != std::find_if(wizards.begin(), wizards.end(),
        [&] (const auto& unit) {
            return is_friend(unit, context.self().getFaction(), context.self().getId()) && is_in_attack_range(unit);
        });

    return !is_any_friend_in_attack_range;
}

bool BaseStrategy::need_apply_fireball(const Context& context, const model::CircularUnit& target) {
    return context.self().getRemainingCooldownTicksByAction()[model::ACTION_FIREBALL] == 0
            && context.self().getMana() >= context.game().getFireballManacost()
            && has_skill(context.self(), model::SKILL_FIREBALL)
            && need_apply_cast(context, target, context.game().getFireballRadius(), context.game().getFireballExplosionMaxDamageRange());
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

bool BaseStrategy::need_apply_cast(const Context& context, const model::CircularUnit& target, double radius, double explosion_radius) {
    const auto direction = Point(1, 0).rotated(normalize_angle(context.self().getAngle() + context.move().getCastAngle()));
    const auto distance = get_position(target).distance(get_position(context.self())) - target.getRadius() - radius + 1;
    const auto cast_target = get_position(context.self()) + direction * std::min(distance, context.self().getCastRange());
    const auto distance_to_target = get_position(target).distance(cast_target);
    const auto lethal_area = radius + target.getRadius();

    if (distance_to_target > lethal_area) {
        return false;
    }

    const Circle cast(get_position(context.self()), radius);
    const auto friend_wizards = filter_friends(get_units<model::Wizard>(context.world()), context.self().getFaction(),
                                               context.self().getId());
    std::vector<Circle> barriers;
    barriers.reserve(friend_wizards.size());
    std::transform(friend_wizards.begin(), friend_wizards.end(), std::back_inserter(barriers), make_circle);

    if (has_intersection_with_barriers(cast, cast_target, barriers)) {
        return false;
    } else if (explosion_radius <= radius) {
        return true;
    }

    const Circle explosion(cast_target, explosion_radius);

    return !has_intersection_with_barriers(explosion, barriers)
            && !explosion.has_intersection(Circle(get_position(context.self()), context.self().getRadius()));
}

}
