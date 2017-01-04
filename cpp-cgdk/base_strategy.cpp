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

class GetTimeDelta {
public:
    template <class Unit>
    GetTimeDelta(const Context& context, const CachedUnit<Unit>& target, model::ProjectileType projectile_type)
        : context_(context),
          projectile_type_(projectile_type),
          unit_position_(get_position(target.value())),
          unit_speed_(target.mean_speed()),
          unit_speed_norm_(unit_speed_.norm()),
          unit_radius_(target.value().getRadius()) {
    }

    double operator ()(double cast_angle) const {
        const auto my_position = get_position(context_.self());
        const auto projectile_speed_norm = get_projectile_speed(projectile_type_, context_.game());
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

        if (distance > unit_radius_ + get_projectile_radius(projectile_type_, context_.game()) - 1) {
            return std::numeric_limits<double>::max();
        }

        return std::abs(projectile_time - unit_time);
    }

    std::pair<bool, Point> get_intersection(double cast_angle) const {
        const auto my_position = get_position(context_.self());
        const auto angle = normalize_angle(context_.self().getAngle() + cast_angle);
        const auto projectile_direction = Point(1, 0).rotated(angle);
        const Line projectile_trajectory(my_position, my_position + projectile_direction);
        const Line unit_trajectory(unit_position_, unit_position_ + unit_speed_);

        return projectile_trajectory.intersection(unit_trajectory);
    }

    const Point& unit_speed() const {
        return unit_speed_;
    }

    double unit_speed_norm() const {
        return unit_speed_norm_;
    }

private:
    const Context& context_;
    model::ProjectileType projectile_type_;
    Point unit_position_;
    Point unit_speed_;
    double unit_speed_norm_;
    double unit_radius_;
};

struct GetCastAction {
    const Context& context;

    template <class Unit>
    std::pair<bool, Action> operator ()(const CachedUnit<Unit>& target, model::ProjectileType projectile_type) const {
        const GetTimeDelta get_time_delta(context, target, projectile_type);
        return (*this)(target.value(), projectile_type, get_time_delta);
    }

    template <class Unit>
    std::pair<bool, Action> operator ()(const Unit& target, model::ProjectileType projectile_type, const GetTimeDelta& get_time_delta) const {
        const auto unit_speed_norm = get_time_delta.unit_speed_norm();

        if (unit_speed_norm == 0) {
            return (*this)(target, projectile_type);
        }

        const auto my_position = get_position(context.self());
        const auto unit_position = get_position(target);
        const auto distance = my_position.distance(unit_position);
        const auto projectile_radius = get_projectile_radius(projectile_type, context.game());
        const auto precision = std::min(projectile_radius / distance * M_1_PI, INVERTED_PHI);
        const std::size_t iterations = std::ceil(std::log(precision) / std::log(INVERTED_PHI));
        const auto current_angle = normalize_angle((unit_position - my_position).absolute_rotation() - context.self().getAngle());
        const auto future_angle = normalize_angle((unit_position + get_time_delta.unit_speed() - my_position).absolute_rotation() - context.self().getAngle());

        double min_cast_angle;
        double max_cast_angle;

        if (current_angle < future_angle) {
            min_cast_angle = std::max(- M_PI / 12, current_angle);
            max_cast_angle = M_PI / 12;
        } else {
            min_cast_angle = - M_PI / 12;
            max_cast_angle = std::min(M_PI / 12, current_angle);
        }

        const auto cast_angle = golden_section(get_time_delta, min_cast_angle, max_cast_angle, iterations);

        bool has_intersection;
        Point intersection;
        std::tie(has_intersection, intersection) = get_time_delta.get_intersection(cast_angle);

        if (!has_intersection) {
            return {false, Action {}};
        }

        const auto distance_to_intersection = my_position.distance(intersection);
        const auto min_cast_distance = distance_to_intersection - get_min_cast_distance_reduce(target);

        if (min_cast_distance > context.self().getCastRange() - 1) {
            return {false, Action {}};
        }

        if (is_friendly_fire(projectile_type, cast_angle, min_cast_distance, intersection)) {
            return {false, Action {}};
        }

        const auto max_cast_distance = projectile_type == model::PROJECTILE_FIREBALL
                ? distance_to_intersection : std::numeric_limits<double>::max();

        return {true, Action {cast_angle, min_cast_distance, max_cast_distance}};
    }

    std::pair<bool, Action> operator ()(const CachedUnit<model::Wizard>& target, model::ProjectileType projectile_type) const {
        return (*this)(target.value(), projectile_type);
    }

    std::pair<bool, Action> operator ()(const model::CircularUnit& target, model::ProjectileType projectile_type) const {
        const auto cast_angle = get_cast_angle_for_static(target);
        const auto my_position = get_position(context.self());
        const auto projectile_radius = get_projectile_radius(projectile_type, context.game());
        const auto direction = Point(1, 0).rotated(normalize_angle(context.self().getAngle() + cast_angle));
        const auto limit = my_position + direction * context.self().getCastRange();
        const Line trajectory(my_position, limit);
        const auto unit_position = get_position(target);
        const auto nearest = trajectory.nearest(unit_position);
        const auto projectile_final_position = nearest.distance(my_position) <= context.self().getCastRange() ? nearest : limit;
        const Circle projectile(my_position, projectile_radius);
        const Circle unit(unit_position, target.getRadius());

        bool has_intersection;
        Point intersection;
        std::tie(has_intersection, intersection) = unit.intersection(projectile, projectile_final_position);

        if (!has_intersection) {
            return {false, Action {}};
        }

        const auto distance_to_intersection = my_position.distance(intersection);
        const auto distance_to_final = my_position.distance(projectile_final_position);

        if (distance_to_final - distance_to_intersection < 1) {
            return {false, Action {}};
        }

        const auto min_cast_distance = distance_to_intersection - projectile_radius;
        const auto projectile_on_intersection = my_position + direction * min_cast_distance;

        if (is_friendly_fire(projectile_type, cast_angle, distance_to_intersection, projectile_on_intersection)) {
            return {false, Action {}};
        }

        const auto max_cast_distance = projectile_type == model::PROJECTILE_FIREBALL
                ? distance_to_intersection : std::numeric_limits<double>::max();

        return {true, Action {cast_angle, min_cast_distance, max_cast_distance}};
    }

    double get_cast_angle_for_static(const model::Unit& target) const {
        const auto angle = context.self().getAngleTo(target);
        return std::min(M_PI / 12, std::max(- M_PI / 12, angle));
    }

    double get_min_cast_distance_for_static(const model::Unit& target) const {
        const auto angle = context.self().getAngleTo(target);
        return std::min(M_PI / 12, std::max(- M_PI / 12, angle));
    }

    double get_min_cast_distance_reduce(const model::CircularUnit& target) const {
        return target.getRadius();
    }

    double get_min_cast_distance_reduce(const model::Wizard& target) const {
        return make_unit_bounds(context, target).max_speed(0);
    }

    bool is_friendly_fire(model::ProjectileType projectile_type, double cast_angle, double min_cast_distance, const Point& final_position) const {
        const auto direction = Point(1, 0).rotated(normalize_angle(context.self().getAngle() + cast_angle));
        const auto initial_position = get_position(context.self()) + direction * min_cast_distance;
        const auto projectile_explosion_radius = get_projectile_explosion_radius(projectile_type, context.game());
        const auto projectile_radius = get_projectile_radius(projectile_type, context.game());

        for (const auto& unit : get_units<model::Wizard>(context.world())) {
            if (unit.getFaction() != context.self().getFaction()) {
                continue;
            }

            const Circle projectile(initial_position, projectile_radius);
            const Circle explosion(initial_position, projectile_explosion_radius);
            const Circle unit_circle(get_position(unit), unit.getRadius());

            if (unit_circle.has_intersection(projectile, final_position)
                    || (projectile_explosion_radius > projectile_radius && unit_circle.has_intersection(explosion))) {
                return true;
            }
        }

        return false;
    }
};

void BaseStrategy::apply_move_and_action(Context& context) {
    if (movement_ != movements_.end()) {
        context.move().setSpeed(movement_->speed());
        context.move().setStrafeSpeed(movement_->strafe_speed());
        context.move().setTurn(movement_->turn());
    }

    const auto actions = get_actions_by_priority_order(context);

    for (const auto action_type : actions) {
        bool need_apply;
        Action action;
        std::tie(need_apply, action) = need_apply_action(context, action_type);

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

    if (!target_.circular_unit(context.cache())) {
        return result;
    }

    if (can_apply_staff(context)) {
        result.push_back(model::ACTION_STAFF);
    }

    if (can_apply_fireball(context)) {
        result.push_back(model::ACTION_FIREBALL);
    }

    if (can_apply_frostbolt(context)) {
        result.push_back(model::ACTION_FROST_BOLT);
    }

    if (can_apply_magic_missile(context)) {
        result.push_back(model::ACTION_MAGIC_MISSILE);
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

bool BaseStrategy::can_apply_haste(const Context& context) {
    return context.self().getRemainingCooldownTicksByAction()[model::ACTION_HASTE] == 0
            && context.self().getMana() >= context.game().getHasteManacost()
            && !is_with_status(context.self(), model::STATUS_HASTENED)
            && has_skill(context.self(), model::SKILL_HASTE);
}

bool BaseStrategy::can_apply_shield(const Context& context) {
    return context.self().getRemainingCooldownTicksByAction()[model::ACTION_SHIELD] == 0
            && context.self().getMana() >= context.game().getShieldManacost()
            && !is_with_status(context.self(), model::STATUS_SHIELDED)
            && has_skill(context.self(), model::SKILL_SHIELD);
}

bool BaseStrategy::can_apply_staff(const Context& context) {
    return context.self().getRemainingCooldownTicksByAction()[model::ACTION_STAFF] == 0;
}

bool BaseStrategy::can_apply_fireball(const Context& context) {
    return context.self().getRemainingCooldownTicksByAction()[model::ACTION_FIREBALL] == 0
            && context.self().getMana() >= context.game().getFireballManacost()
            && has_skill(context.self(), model::SKILL_FIREBALL);
}

bool BaseStrategy::can_apply_frostbolt(const Context& context) {
    return context.self().getRemainingCooldownTicksByAction()[model::ACTION_FROST_BOLT] == 0
            && context.self().getMana() >= context.game().getFrostBoltManacost()
            && has_skill(context.self(), model::SKILL_FROST_BOLT);
}

bool BaseStrategy::can_apply_magic_missile(const Context& context) {
    return context.self().getRemainingCooldownTicksByAction()[model::ACTION_MAGIC_MISSILE] == 0
            && context.self().getMana() >= context.game().getMagicMissileManacost();
}

std::pair<bool, Action> BaseStrategy::need_apply_action(const Context& context, model::ActionType type) const {
    switch (type) {
        case model::ACTION_STAFF:
            return need_apply_staff(context);
        case model::ACTION_MAGIC_MISSILE:
            return need_apply_magic_missile(context);
        case model::ACTION_FROST_BOLT:
            return need_apply_frostbolt(context);
        case model::ACTION_FIREBALL:
            return need_apply_fireball(context);
        case model::ACTION_HASTE:
            return need_apply_haste(context);
        case model::ACTION_SHIELD:
            return need_apply_shield(context);
        default:
            return {false, Action {}};
    }
}

std::pair<bool, Action> BaseStrategy::need_apply_haste(const Context&) const {
    return {true, Action {}};
}

std::pair<bool, Action> BaseStrategy::need_apply_shield(const Context&) const {
    return {true, Action {}};
}

std::pair<bool, Action> BaseStrategy::need_apply_staff(const Context& context) const {
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

    if (this->target_.is<model::Bonus>()) {
        return {false, Action {}};
    }

    const auto target = target_.circular_unit(context.cache());

    if (!target) {
        return {false, Action {}};
    }

    if (!is_in_attack_range(*target)) {
        return {false, Action {}};
    }

    const auto wizards = get_units<model::Wizard>(context.world());
    const auto is_any_friend_in_attack_range = wizards.end() != std::find_if(wizards.begin(), wizards.end(),
        [&] (const auto& unit) {
            return is_friend(unit, context.self().getFaction(), context.self().getId()) && is_in_attack_range(unit);
        });

    return {!is_any_friend_in_attack_range, Action {}};
}

std::pair<bool, Action> BaseStrategy::need_apply_magic_missile(const Context& context) const {
    return need_apply_cast(context, model::PROJECTILE_MAGIC_MISSILE);
}

std::pair<bool, Action> BaseStrategy::need_apply_fireball(const Context& context) const {
    return need_apply_cast(context, model::PROJECTILE_FIREBALL);
}

std::pair<bool, Action> BaseStrategy::need_apply_frostbolt(const Context& context) const {
    if (target_.is<model::Building>() || target_.is<model::Tree>()) {
        return {false, Action {}};
    }
    return need_apply_cast(context, model::PROJECTILE_FROST_BOLT);
}

std::pair<bool, Action> BaseStrategy::need_apply_cast(const Context& context, model::ProjectileType projectile_type) const {
    if (this->target_.is<model::Bonus>() || !target_.circular_unit(context.cache())) {
        return {false, Action {}};
    }

    const GetCastAction get_cast_action {context};

    return  target_.apply_cached(context.cache(),
        [&] (const auto& v) { return get_cast_action(*v, projectile_type); });
}

}
