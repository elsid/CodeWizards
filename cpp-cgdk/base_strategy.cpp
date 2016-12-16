#include "base_strategy.hpp"
#include "optimal_destination.hpp"
#include "optimal_position.hpp"
#include "golden_section.hpp"

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
    apply_move_and_action(context);
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
    if (context.self().getLife() < context.self().getMaxLife() / 2 && mode_ != move_mode_) {
        return use_retreat_mode();
    }

    double max_distance = get_max_distance_for_unit_candidate(context);

    if (mode_ == battle_mode_) {
        max_distance = context.game().getStaffRange() + (max_distance - context.game().getStaffRange())
                * line_factor(mode_ticks_, BATTLE_MODE_TICKS, BATTLE_MODE_TICKS - context.game().getWizardActionCooldownTicks());
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

class GetTimeDelta {
public:
    template <class Unit>
    GetTimeDelta(const Context& context, const CachedUnit<Unit>& target, model::ActionType action)
        : context_(context),
          action_(action),
          unit_position_(get_position(target.value())),
          unit_speed_(target.mean_speed()),
          unit_speed_norm_(unit_speed_.norm()) {
    }

    double operator ()(double cast_angle) const {
        const auto my_position = get_position(context_.self());
        const auto projectile_type = get_projectile_type_by_action(action_);
        const auto projectile_speed_norm = get_projectile_speed(projectile_type, context_.game());
        const auto angle = normalize_angle(context_.self().getAngle() + cast_angle);
        const auto projectile_direction = Point(1, 0).rotated(angle);
        const Line projectile_trajectory(my_position, my_position + projectile_direction);
        const Line unit_trajectory(unit_position_, unit_position_ + unit_speed_);

        bool has_intersection;
        Point intersection;
        std::tie(has_intersection, intersection) = projectile_trajectory.intersection(unit_trajectory);
        if (!has_intersection) {
            return std::numeric_limits<double>::max();
        }

        const auto projectile_time = intersection.distance(my_position) / projectile_speed_norm;
        const auto projectile_path_length = std::min(projectile_speed_norm * projectile_time,
                                                     context_.self().getCastRange());
        const auto projectile_target = my_position + projectile_direction * projectile_path_length;
        const auto unit_time = intersection.distance(unit_position_) / unit_speed_norm_;
        const auto unit_target = unit_position_ + unit_speed_ * unit_time;
        const auto distance = projectile_target.distance(unit_target);

        if (distance > 1) {
            return std::numeric_limits<double>::max();
        }

        return std::abs(projectile_time - unit_time);
    }

    double unit_speed_norm() const {
        return unit_speed_norm_;
    }

private:
    const Context& context_;
    model::ActionType action_;
    Point unit_position_;
    Point unit_speed_;
    double unit_speed_norm_;
};

struct ApplyCast {
    Context& context;

    template <class Unit>
    bool operator ()(const CachedUnit<Unit>& target, model::ActionType action) {
        const GetTimeDelta get_time_delta(context, target, action);
        return (*this)(target.value(), action, get_time_delta);
    }

    template <class Unit>
    bool operator ()(const Unit& target, model::ActionType action, const GetTimeDelta& get_time_delta) {
        const auto unit_speed_norm = get_time_delta.unit_speed_norm();
        const auto projectile_type = get_projectile_type_by_action(action);
        const auto projectile_radius = get_projectile_radius(projectile_type, context.game());
        double cast_angle;
        bool apply = false;

        if (unit_speed_norm == 0) {
            cast_angle = get_cast_angle_for_static(target);
            const auto direction = Point(1, 0).rotated(normalize_angle(context.self().getAngle() + cast_angle));
            const auto my_position = get_position(context.self());
            const Line trajectory(my_position, my_position + direction * context.self().getCastRange());
            const auto unit_position = get_position(target);
            const auto projectile_target = trajectory.nearest(unit_position);
            const auto distance_to_target = projectile_target.distance(unit_position);

            if (distance_to_target < 1 && trajectory.has_point(projectile_target)) {
                apply = true;
            }
        } else {
            const auto distance = get_position(context.self()).distance(get_position(target));
            const auto precision = std::min(projectile_radius / distance * M_1_PI, INVERTED_PHI);
            const std::size_t iterations = std::ceil(std::log(precision) / std::log(INVERTED_PHI));
            cast_angle = golden_section(get_time_delta, - M_PI / 12, M_PI / 12, iterations);
            const auto time_delta = get_time_delta(cast_angle);

            if (time_delta < 1) {
                apply = true;
            }
        }

        if (apply) {
            const auto distance = get_position(context.self()).distance(get_position(target));
            const auto type = get_projectile_type_by_action(action);
            const auto radius = get_projectile_radius(type, context.game());
            context.move().setCastAngle(cast_angle);
            context.move().setMinCastDistance(distance - target.getRadius() - radius);
            context.move().setAction(action);
            return true;
        }

        return false;
    }

    bool operator ()(const CachedUnit<model::Wizard>& target, model::ActionType action) {
        return (*this)(target.value(), action);
    }

    bool operator ()(const model::Wizard& target, model::ActionType action) {
        const auto cast_angle = get_cast_angle_for_static(target);
        const auto direction = Point(1, 0).rotated(normalize_angle(context.self().getAngle() + cast_angle));
        const auto my_position = get_position(context.self());
        const Line trajectory(my_position, my_position + direction * context.self().getCastRange());
        const auto unit_position = get_position(target);
        const auto projectile_target = trajectory.nearest(unit_position);
        const auto projectile_type = get_projectile_type_by_action(action);
        const auto projectile_speed_norm = get_projectile_speed(projectile_type, context.game());
        const auto projectile_time = projectile_target.distance(my_position) / projectile_speed_norm;
        const auto unit_bounds = make_unit_bounds(context, target);
        const auto future_max_distance =  unit_position.distance(projectile_target) + unit_bounds.max_speed(0) * projectile_time;
        const auto projectile_radius = get_projectile_radius(projectile_type, context.game());

        if (future_max_distance < target.getRadius() + projectile_radius) {
            const auto distance = get_position(context.self()).distance(get_position(target));
            const auto type = get_projectile_type_by_action(action);
            const auto radius = get_projectile_radius(type, context.game());
            context.move().setCastAngle(cast_angle);
            context.move().setMinCastDistance(distance - target.getRadius() - radius);
            context.move().setAction(action);
            return true;
        }

        return false;
    }

    double get_cast_angle_for_static(const model::Unit& target) const {
        const auto angle = context.self().getAngleTo(target);
        return std::min(M_PI / 12, std::max(- M_PI / 12, angle));
    }
};

void BaseStrategy::apply_move_and_action(Context& context) {
    if (movement_ != movements_.end()) {
        context.move().setSpeed(movement_->speed());
        context.move().setStrafeSpeed(movement_->strafe_speed());
        context.move().setTurn(movement_->turn());
    }

    const auto actions = get_actions_by_priority_order(context);

    if (!actions.empty()) {
        context.move().setAction(actions.front());
    }

    if (!target_.is_some()) {
        return;
    }

    const auto target = target_.circular_unit(context.cache());

    if (!target) {
        return;
    }

    if (this->target_.is<model::Bonus>()) {
        return;
    }

    context.move().setAction(model::ACTION_NONE);

    for (const auto action : actions) {
        if (action != model::ACTION_MAGIC_MISSILE && action != model::ACTION_FIREBALL && action != model::ACTION_FROST_BOLT) {
            context.move().setAction(action);
            break;
        }

        ApplyCast apply_cast {context};

        const auto applied = target_.apply_cached(context.cache(),
            [&] (const auto target) { return apply_cast(*target, action); });

        if (applied) {
            break;
        }
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

std::vector<model::ActionType> BaseStrategy::get_actions_by_priority_order(Context& context) const {
    std::vector<model::ActionType> result;
    result.reserve(model::_ACTION_COUNT_);

    if (context.self().getRemainingActionCooldownTicks() > 0) {
        return result;
    }

    if (can_apply_haste(context)) {
        result.push_back(model::ACTION_HASTE);
    }

    if (can_apply_shield(context)) {
        result.push_back(model::ACTION_SHIELD);
    }

    if (target_.is<model::Bonus>()) {
        return result;
    }

    if (const auto target = target_.circular_unit(context.cache())) {
        if (can_apply_staff(context, *target)) {
            result.push_back(model::ACTION_STAFF);
        } else if (can_apply_fireball(context, *target)) {
            result.push_back(model::ACTION_FIREBALL);
        } else if (can_apply_frostbolt(context, *target)) {
            result.push_back(model::ACTION_FROST_BOLT);
        } else if (can_apply_magic_missile(context, *target)) {
            result.push_back(model::ACTION_MAGIC_MISSILE);
        }
    }

    return result;
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

bool BaseStrategy::can_apply_haste(const Context& context) const {
    return context.self().getRemainingCooldownTicksByAction()[model::ACTION_HASTE] == 0
            && context.self().getMana() >= context.game().getHasteManacost()
            && !is_with_status(context.self(), model::STATUS_HASTENED)
            && has_skill(context.self(), model::SKILL_HASTE)
            && ((!target_.is<model::Bonus>()
                && target_.is_some())
                || get_position(context.self()).distance(destination_) > 0.5 * context.self().getVisionRange());
}

bool BaseStrategy::can_apply_shield(const Context& context) const {
    return context.self().getRemainingCooldownTicksByAction()[model::ACTION_SHIELD] == 0
            && context.self().getMana() >= context.game().getShieldManacost()
            && !is_with_status(context.self(), model::STATUS_SHIELDED)
            && !target_.is<model::Bonus>()
            && target_.is_some()
            && has_skill(context.self(), model::SKILL_SHIELD);
}

bool BaseStrategy::can_apply_staff(const Context& context, const model::CircularUnit& target) {
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

bool BaseStrategy::can_apply_fireball(const Context& context, const model::CircularUnit& target) {
    return context.self().getRemainingCooldownTicksByAction()[model::ACTION_FIREBALL] == 0
            && context.self().getMana() >= context.game().getFireballManacost()
            && has_skill(context.self(), model::SKILL_FIREBALL)
            && can_apply_cast(context, target, context.game().getFireballRadius(),
                               context.game().getFireballExplosionMinDamageRange());
}

bool BaseStrategy::can_apply_frostbolt(const Context& context, const model::CircularUnit& target) {
    return context.self().getRemainingCooldownTicksByAction()[model::ACTION_FROST_BOLT] == 0
            && context.self().getMana() >= context.game().getFrostBoltManacost()
            && has_skill(context.self(), model::SKILL_FROST_BOLT)
            && can_apply_cast(context, target, context.game().getFrostBoltRadius());
}

bool BaseStrategy::can_apply_magic_missile(const Context& context, const model::CircularUnit& target) {
    return context.self().getRemainingCooldownTicksByAction()[model::ACTION_MAGIC_MISSILE] == 0
            && context.self().getMana() >= context.game().getMagicMissileManacost()
            && can_apply_cast(context, target, context.game().getMagicMissileRadius());
}

bool BaseStrategy::can_apply_cast(const Context& context, const model::CircularUnit& target, double radius, double explosion_radius) {
    const auto direction = Point(1, 0).rotated(normalize_angle(context.self().getAngle() + context.move().getCastAngle()));
    const auto distance = get_position(target).distance(get_position(context.self())) - target.getRadius() - radius + 1;
    const auto cast_target = get_position(context.self()) + direction * std::min(distance, context.self().getCastRange());

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
