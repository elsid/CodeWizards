#include "action.hpp"
#include "golden_section.hpp"
#include "optimal_movement.hpp"
#include "optimal_target.hpp"

namespace strategy {

bool can_apply_haste(const Context& context) {
    return context.self().getRemainingCooldownTicksByAction()[model::ACTION_HASTE] == 0
            && context.self().getMana() >= context.game().getHasteManacost()
            && !is_with_status(context.self(), model::STATUS_HASTENED)
            && has_skill(context.self(), model::SKILL_HASTE);
}

bool can_apply_shield(const Context& context) {
    return context.self().getRemainingCooldownTicksByAction()[model::ACTION_SHIELD] == 0
            && context.self().getMana() >= context.game().getShieldManacost()
            && !is_with_status(context.self(), model::STATUS_SHIELDED)
            && has_skill(context.self(), model::SKILL_SHIELD);
}

bool can_apply_staff(const Context& context) {
    return context.self().getRemainingCooldownTicksByAction()[model::ACTION_STAFF] == 0;
}

bool can_apply_fireball(const Context& context) {
    return context.self().getRemainingCooldownTicksByAction()[model::ACTION_FIREBALL] == 0
            && context.self().getMana() >= context.game().getFireballManacost()
            && has_skill(context.self(), model::SKILL_FIREBALL);
}

bool can_apply_frostbolt(const Context& context) {
    return context.self().getRemainingCooldownTicksByAction()[model::ACTION_FROST_BOLT] == 0
            && context.self().getMana() >= context.game().getFrostBoltManacost()
            && has_skill(context.self(), model::SKILL_FROST_BOLT);
}

bool can_apply_magic_missile(const Context& context) {
    return context.self().getRemainingCooldownTicksByAction()[model::ACTION_MAGIC_MISSILE] == 0
            && context.self().getMana() >= context.game().getMagicMissileManacost();
}

std::vector<model::ActionType> get_actions_by_priority_order(const Context& context, const Target& target) {
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

    if (target.is<model::Bonus>()) {
        return result;
    }

    if (!target.circular_unit(context.cache())) {
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

std::pair<bool, Action> need_apply_status(const Context& context, model::StatusType status) {
    const auto my_position = get_position(context.self());
    const model::Wizard* target = nullptr;

    for (const auto &unit : context.world().getWizards()) {
        if (unit.getFaction() == context.self().getFaction()
                && !unit.isMe()
                && get_position(unit).distance(my_position) <= context.self().getCastRange()
                && !is_with_status(unit, status)
                && (!target || unit.getLife() < target->getLife())) {
            target = &unit;
        }
    }

    return {true, (target ? Action(target->getId()) : Action())};
}

std::pair<bool, Action> need_apply_haste(const Context& context) {
    return need_apply_status(context, model::STATUS_HASTENED);
}

std::pair<bool, Action> need_apply_shield(const Context& context) {
    return need_apply_status(context, model::STATUS_SHIELDED);
}

std::pair<bool, Action> need_apply_staff(const Context& context, const Target& target) {
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

    if (target.is<model::Bonus>()) {
        return {false, Action()};
    }

    const auto target_unit = target.circular_unit(context.cache());

    if (!target_unit) {
        return {false, Action()};
    }

    if (!is_in_attack_range(*target_unit)) {
        return {false, Action()};
    }

    const auto wizards = get_units<model::Wizard>(context.world());
    const auto is_any_friend_in_attack_range = wizards.end() != std::find_if(wizards.begin(), wizards.end(),
        [&] (const auto& unit) {
            return is_friend(unit, context.self().getFaction()) && is_in_attack_range(unit);
        });

    return {!is_any_friend_in_attack_range, Action()};
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
            return {false, Action()};
        }

        const auto distance_to_intersection = my_position.distance(intersection);
        const auto min_cast_distance = distance_to_intersection - get_min_cast_distance_reduce(target);

        if (min_cast_distance > context.self().getCastRange() - 1) {
            return {false, Action()};
        }

        if (is_friendly_fire(projectile_type, cast_angle, min_cast_distance, intersection)) {
            return {false, Action()};
        }

        const auto max_cast_distance = projectile_type == model::PROJECTILE_FIREBALL
                ? distance_to_intersection : std::numeric_limits<double>::max();

        return {true, Action(cast_angle, min_cast_distance, max_cast_distance)};
    }

    std::pair<bool, Action> operator ()(const CachedUnit<model::Wizard>& target, model::ProjectileType projectile_type) const {
        const GetMaxDamage get_max_damage {context};
        const auto my_position = get_position(context.self());
        const auto unit_position = get_position(target.value());
        const auto distance = my_position.distance(unit_position);
        const auto unit_action_tick = get_max_damage.next_attack_action(target.value(), distance + 2 * context.self().getRadius()).second;
        const auto ticks = std::ceil(distance / get_projectile_speed(projectile_type, context.game()));
        const auto cast_angle = get_cast_angle_for_static(target.value());
        const auto direction = Point(1, 0).rotated(normalize_angle(context.self().getAngle() + cast_angle));
        const auto limit = my_position + direction * context.self().getCastRange();
        const Line trajectory(my_position, limit);
        const auto nearest = trajectory.nearest(unit_position);
        const auto has_point = trajectory.has_point(nearest);

        if (!has_point || nearest.distance(unit_position) > 1e-3) {
            return {false, Action()};
        }

        if (ticks <= unit_action_tick + 1) {
            const auto unit_bounds = make_unit_bounds(context, target.value());
            const auto unit_speed = (unit_bounds.max_speed(0) - unit_bounds.min_speed(0) + 2 * unit_bounds.max_strafe_speed(0)) / 4;
            const auto unit_path_length = ticks * unit_speed;
            const auto range = unit_path_length + distance;
            const auto max_range = context.self().getCastRange() + target.value().getRadius() + get_projectile_radius(projectile_type, context.game());

            if (range >  max_range) {
                return {false, Action()};
            }
        }

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
        const auto has_point = trajectory.has_point(nearest);
        const auto projectile_on_collision = has_point ? nearest : limit;
        const auto distance = projectile_on_collision.distance(unit_position);
        const auto radius_sum = projectile_radius + target.getRadius();

        if (distance > radius_sum) {
            return {false, Action()};
        }

        const auto distance_to_collision = my_position.distance(projectile_on_collision);

        if (is_friendly_fire(projectile_type, cast_angle, distance_to_collision, projectile_on_collision)) {
            return {false, Action()};
        }

        const auto max_cast_distance = projectile_type == model::PROJECTILE_FIREBALL
                ? distance_to_collision : std::numeric_limits<double>::max();

        return {true, Action(cast_angle, distance_to_collision, max_cast_distance)};
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

std::pair<bool, Action> need_apply_cast(const Context& context, const Target& target, model::ProjectileType projectile_type) {
    if (target.is<model::Bonus>() || !target.circular_unit(context.cache())) {
        return {false, Action()};
    }

    const GetCastAction get_cast_action {context};

    return target.apply_cached(context.cache(),
        [&] (const auto& v) { return get_cast_action(*v, projectile_type); });
}

std::pair<bool, Action> need_apply_magic_missile(const Context& context, const Target& target) {
    return need_apply_cast(context, target, model::PROJECTILE_MAGIC_MISSILE);
}

std::pair<bool, Action> need_apply_fireball(const Context& context, const Target& target) {
    return need_apply_cast(context, target, model::PROJECTILE_FIREBALL);
}

std::pair<bool, Action> need_apply_frostbolt(const Context& context, const Target& target) {
    if (target.is<model::Building>() || target.is<model::Tree>()) {
        return {false, Action()};
    }
    return need_apply_cast(context, target, model::PROJECTILE_FROST_BOLT);
}

std::pair<bool, Action> need_apply_action(const Context& context, const Target& target, model::ActionType type) {
    switch (type) {
        case model::ACTION_STAFF:
            return need_apply_staff(context, target);
        case model::ACTION_MAGIC_MISSILE:
            return need_apply_magic_missile(context, target);
        case model::ACTION_FROST_BOLT:
            return need_apply_frostbolt(context, target);
        case model::ACTION_FIREBALL:
            return need_apply_fireball(context, target);
        case model::ACTION_HASTE:
            return need_apply_haste(context);
        case model::ACTION_SHIELD:
            return need_apply_shield(context);
        default:
            return {false, Action()};
    }
}

}
