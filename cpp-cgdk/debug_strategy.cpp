#include "debug_strategy.hpp"
#include "battle_mode.hpp"
#include "optimal_destination.hpp"
#include "helpers.hpp"

namespace strategy {

static const std::unordered_map<model::LaneType, int> LANES_COLORS = {
    {model::LANE_BOTTOM, 0xFF0000},
    {model::LANE_MIDDLE, 0x00FF00},
    {model::LANE_TOP, 0x0000FF},
};

static const std::unordered_map<model::ActionType, int> ACTIONS_COLORS = {
    {model::ACTION_STAFF, 0x00FF00},
    {model::ACTION_MAGIC_MISSILE, 0xFF00FF},
    {model::ACTION_FROST_BOLT, 0x4444FF},
    {model::ACTION_FIREBALL, 0xFF0000},
    {model::ACTION_HASTE, 0xFFFF00},
    {model::ACTION_SHIELD, 0x0000FF},
};

static const std::unordered_map<model::SkillType, std::string> SKILLS_NAMES({
    {model::SKILL_RANGE_BONUS_PASSIVE_1, "RANGE_BONUS_PASSIVE_1"},
    {model::SKILL_RANGE_BONUS_AURA_1, "RANGE_BONUS_AURA_1"},
    {model::SKILL_RANGE_BONUS_PASSIVE_2, "RANGE_BONUS_PASSIVE_2"},
    {model::SKILL_RANGE_BONUS_AURA_2, "RANGE_BONUS_AURA_2"},
    {model::SKILL_ADVANCED_MAGIC_MISSILE, "ADVANCED_MAGIC_MISSILE"},
    {model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_1, "MAGICAL_DAMAGE_BONUS_PASSIVE_1"},
    {model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_1, "MAGICAL_DAMAGE_BONUS_AURA_1"},
    {model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_2, "MAGICAL_DAMAGE_BONUS_PASSIVE_2"},
    {model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_2, "MAGICAL_DAMAGE_BONUS_AURA_2"},
    {model::SKILL_FROST_BOLT, "FROST_BOLT"},
    {model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_1, "STAFF_DAMAGE_BONUS_PASSIVE_1"},
    {model::SKILL_STAFF_DAMAGE_BONUS_AURA_1, "STAFF_DAMAGE_BONUS_AURA_1"},
    {model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_2, "STAFF_DAMAGE_BONUS_PASSIVE_2"},
    {model::SKILL_STAFF_DAMAGE_BONUS_AURA_2, "STAFF_DAMAGE_BONUS_AURA_2"},
    {model::SKILL_FIREBALL, "FIREBALL"},
    {model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_1, "MOVEMENT_BONUS_FACTOR_PASSIVE_1"},
    {model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_1, "MOVEMENT_BONUS_FACTOR_AURA_1"},
    {model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_2, "MOVEMENT_BONUS_FACTOR_PASSIVE_2"},
    {model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_2, "MOVEMENT_BONUS_FACTOR_AURA_2"},
    {model::SKILL_HASTE, "HASTE"},
    {model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_1, "MAGICAL_DAMAGE_ABSORPTION_PASSIVE_1"},
    {model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_1, "MAGICAL_DAMAGE_ABSORPTION_AURA_1"},
    {model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_2, "MAGICAL_DAMAGE_ABSORPTION_PASSIVE_2"},
    {model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_2, "MAGICAL_DAMAGE_ABSORPTION_AURA_2"},
    {model::SKILL_SHIELD, "SHIELD"},
});

struct Pre {
    Debug& debug;

    Pre(Debug& debug) : debug(debug) {
        debug.beginPre();
    }

    ~Pre() {
        debug.endPre();
    }
};

struct Post {
    Debug& debug;

    Post(Debug& debug) : debug(debug) {
        debug.beginPost();
    }

    ~Post() {
        debug.endPost();
    }
};

struct Abs {
    Debug& debug;

    Abs(Debug& debug) : debug(debug) {
        debug.beginAbs();
    }

    ~Abs() {
        debug.endAbs();
    }
};

void DebugStrategy::apply(Context& context) {
    component_->apply(context);
    visualize(context);

    if (context.move().getSkillToLearn() != model::_SKILL_UNKNOWN_) {
        SLOG(context) << "learn_skill " << SKILLS_NAMES.at(context.move().getSkillToLearn())
                      << " at level " << context.self().getLevel() << '\n';
    }
}

void DebugStrategy::visualize(const Context& context) {
    Post post(debug_);
    visualize_graph(context);
    visualize_graph_path(context);
    visualize_positions_penalties(context);
    visualize_points(context);
    visualize_path(context);
    visualize_ticks_states(context);
    visualize_states(context);
    visualize_destination(context);
    visualize_target(context);
    visualize_units(context);
}

void DebugStrategy::visualize_graph(const Context& context) {
    const GetNodeScore get_node_score(context, base_.graph(), base_.move_mode().target_lane(), context.self());
    const auto& nodes = base_.graph().nodes();
    const auto& arcs = base_.graph().arcs();
    Matrix visualized_arcs(nodes.size(), 0);
    for (const auto& src : nodes) {
        for (const auto& dst : nodes) {
            if (arcs.get(src.first, dst.first) != std::numeric_limits<double>::max()
                    && !visualized_arcs.get(src.first, dst.first)) {
                debug_.line(src.second.position.x(), src.second.position.y(), dst.second.position.x(), dst.second.position.y(), 0xAAAAAA);
                visualized_arcs.set(src.first, dst.first, 1);
                visualized_arcs.set(dst.first, src.first, 1);
            }
        }
    }
    std::vector<double> scores;
    scores.reserve(nodes.size());
    std::transform(nodes.begin(), nodes.end(), std::back_inserter(scores),
        [&] (const auto& v) { return get_node_score(v.second); });
    const auto minmax_score = std::minmax_element(scores.begin(), scores.end());
    const auto interval = *minmax_score.second - *minmax_score.first;
    for (const auto& node : nodes) {
        const auto score = scores[node.first];
        const auto color = get_color((score - *minmax_score.first) / (interval ? interval : 1));
        debug_.fillCircle(node.second.position.x(), node.second.position.y(), 10, color);
        debug_.text(node.second.position.x() + 30, node.second.position.y() + 30, std::to_string(node.first).c_str(), 0xAAAAAA);
        debug_.text(node.second.position.x() + 30, node.second.position.y() - 30, std::to_string(score).c_str(), color);
        const auto lane_type = std::find_if(LANES_COLORS.begin(), LANES_COLORS.end(),
            [&] (auto lane) { return base_.graph().lanes_nodes().at(lane.first).count(node.first); });
        if (lane_type != LANES_COLORS.end()) {
            debug_.circle(node.second.position.x(), node.second.position.y(), 15, lane_type->second);
        }
    }
    const auto friend_base = nodes.at(base_.graph().friend_base());
    debug_.circle(friend_base.position.x(), friend_base.position.y(), 40, 0x00AA00);
    const auto enemy_base = nodes.at(base_.graph().enemy_base());
    debug_.circle(enemy_base.position.x(), enemy_base.position.y(), 40, 0xAA0000);
}

void DebugStrategy::visualize_graph_path(const Context& context) {
    Point prev = get_position(context.self());
    for (auto node = base_.move_mode().path_node(); node != base_.move_mode().path().end(); ++node) {
        const auto position = node->position;
        debug_.line(prev.x(), prev.y(), position.x(), position.y(), 0x222222);
        debug_.circle(position.x(), position.y(), 20, 0x222222);
        prev = position;
    }
    const auto& destination = base_.move_mode().destination();
    if (destination.first) {
        const auto& position = destination.second.position;
        debug_.circle(position.x(), position.y(), 30, 0x222222);
    }
}

void DebugStrategy::visualize_points(const Context& context) {
    const auto& points = base_.battle_mode().points();

    if (points.empty()) {
        return;
    }

    const auto minmax = std::minmax_element(points.begin(), points.end(),
        [&] (const auto& lhs, const auto& rhs) { return lhs.second < rhs.second; });
    const auto interval = minmax.second->second - minmax.first->second;
    auto prev = get_position(context.self());
    for (const auto& v : base_.battle_mode().points()) {
        const auto color = get_color((v.second - minmax.first->second) / (interval ? interval : 1));
        const auto& point = v.first;
        debug_.line(prev.x(), prev.y(), point.x(), point.y(), color);
        debug_.fillCircle(point.x(), point.y(), 4, color);
        prev = point;
    }
}

void DebugStrategy::visualize_path(const Context& context) {
    auto prev = get_position(context.self());
    for (const auto& point : base_.path()) {
        debug_.line(prev.x(), prev.y(), point.x(), point.y(), 0x000099);
        debug_.fillCircle(point.x(), point.y(), 3, 0x000099);
        prev = point;
    }
}

void DebugStrategy::visualize_positions_penalties(const Context& context) {
    if (const auto target = base_.target().unit<model::Bonus>(context.cache())) {
        visualize_positions_penalties(context, target);
    } else if (const auto target = base_.target().unit<model::Building>(context.cache())) {
        visualize_positions_penalties(context, target);
    } else if (const auto target = base_.target().unit<model::Minion>(context.cache())) {
        visualize_positions_penalties(context, target);
    } else if (const auto target = base_.target().unit<model::Wizard>(context.cache())) {
        visualize_positions_penalties(context, target);
    } else if (const auto target = base_.target().unit<model::Tree>(context.cache())) {
        visualize_positions_penalties(context, target);
    } else if (&base_.mode() != &base_.move_mode()) {
        visualize_positions_penalties(context, static_cast<const model::LivingUnit*>(nullptr));
    }
}

void DebugStrategy::visualize_destination(const Context& /*context*/) {
    debug_.line(base_.destination().x() - 35, base_.destination().y() - 35,
                base_.destination().x() + 35, base_.destination().y() + 35, 0x0000FF);
    debug_.line(base_.destination().x() + 35, base_.destination().y() - 35,
                base_.destination().x() - 35, base_.destination().y() + 35, 0x0000FF);
}

void DebugStrategy::visualize_target(const Context& context) {
    if (const auto target = base_.target().circular_unit(context.cache())) {
        debug_.circle(target->getX(), target->getY(), target->getRadius() + 20, 0xAA0000);
    }
}

void DebugStrategy::visualize_units(const Context& context) {
    const GetTargetScore get_target_score {context};
    max_target_score = 0;
    min_target_score = std::numeric_limits<double>::max();
    for (const auto& unit : get_units<model::Wizard>(context.cache())) {
        if (unit.second.value().getFaction() != context.self().getFaction()) {
            max_target_score = std::max(max_target_score, get_target_score(unit.second.value()));
            min_target_score = std::max(min_target_score, get_target_score(unit.second.value()));
        }
    }
    for (const auto& unit : get_units<model::Building>(context.cache())) {
        if (unit.second.value().getFaction() != context.self().getFaction()) {
            max_target_score = std::max(max_target_score, get_target_score(unit.second.value()));
            min_target_score = std::max(min_target_score, get_target_score(unit.second.value()));
        }
    }
    for (const auto& unit : get_units<model::Minion>(context.cache())) {
        if (unit.second.value().getFaction() != context.self().getFaction()) {
            max_target_score = std::max(max_target_score, get_target_score(unit.second.value()));
            min_target_score = std::max(min_target_score, get_target_score(unit.second.value()));
        }
    }
    for (const auto& unit : get_units<model::Wizard>(context.cache())) {
        visualize_unit(context, unit.second.value());
    }
    for (const auto& unit : get_units<model::Building>(context.cache())) {
        visualize_unit(context, unit.second.value());
    }
    for (const auto& unit : get_units<model::Minion>(context.cache())) {
        visualize_unit(context, unit.second.value());
    }
    for (const auto& unit : get_units<model::Tree>(context.cache())) {
        visualize_unit(context, unit.second.value());
    }
    for (const auto& unit : get_units<model::Projectile>(context.cache())) {
        visualize_unit(context, unit.second.value());
    }
}

void DebugStrategy::visualize_unit(const Context& context, const model::Wizard& unit) {
    const GetMaxDamage get_max_damage {context};
    const GetAttackRange get_attack_range {context};
    const auto range = get_attack_range(unit, get_max_damage.next_attack_action(unit, context.game().getMapSize()).first);
    debug_.circle(unit.getX(), unit.getY(), unit.getVisionRange(), 0x44FF44);
    debug_.circle(unit.getX(), unit.getY(), unit.getCastRange(), 0xFF4444);
    debug_.circle(unit.getX(), unit.getY(), context.game().getStaffRange(), 0xFF2222);
    const auto missile_direction_left = Point(1, 0).rotated(normalize_angle(unit.getAngle() - context.game().getStaffSector() / 2));
    const auto missile_direction_right = Point(1, 0).rotated(normalize_angle(unit.getAngle() + context.game().getStaffSector() / 2));
    const auto missile_max_target_left = get_position(unit) + missile_direction_left * range;
    const auto missile_max_target_right = get_position(unit) + missile_direction_right * range;
    debug_.line(unit.getX(), unit.getY(), missile_max_target_left.x(), missile_max_target_left.y(), 0xFF4444);
    debug_.line(unit.getX(), unit.getY(), missile_max_target_right.x(), missile_max_target_right.y(), 0xFF4444);
    debug_.arc(unit.getX(), unit.getY(), range, normalize_angle(unit.getAngle() - context.game().getStaffSector() / 2),
               context.game().getStaffSector(), 0xFF4444);
    debug_.text(unit.getX() + unit.getRadius(), unit.getY() + unit.getRadius(),
                std::to_string(unit.getRemainingActionCooldownTicks()).c_str(), 0x444444);

    visualize_unit_speed(unit);
    visualize_unit_mean_speed(context, unit);
    visualize_unit_mean_life_change_speed(context, unit);

    for (int action = model::ACTION_STAFF, shift = 0; action < model::_ACTION_COUNT_; ++action) {
        const auto skill = ACTIONS_SKILLS.at(action);
        if (skill != model::_SKILL_UNKNOWN_ && !has_skill(unit, skill)) {
            continue;
        }
        debug_.text(unit.getX() + unit.getRadius() + 35 * (1 + shift), unit.getY() + unit.getRadius(),
                    std::to_string(unit.getRemainingCooldownTicksByAction()[action]).c_str(), ACTIONS_COLORS.at(model::ActionType(action)));
        ++shift;
    }

    int shift = 1;
    for (const auto skill : unit.getSkills()) {
        debug_.text(unit.getX() + unit.getRadius(), unit.getY() - unit.getRadius() - 20 * shift,
                    SKILLS_NAMES.at(skill).c_str(), 0x444444);
        ++shift;
    }

    if (unit.getFaction() == context.self().getFaction()) {
        return;
    }

    const GetTargetScore get_target_score {context};
    const auto score = get_target_score(unit);
    debug_.text(unit.getX() + unit.getRadius(), unit.getY() - unit.getRadius(),
                std::to_string(score).c_str(), get_color(score / (max_target_score ? max_target_score : 1.0)));
}

void DebugStrategy::visualize_unit(const Context& context, const model::Building& unit) {
    double attack_range = unit.getType() == model::BUILDING_GUARDIAN_TOWER
            ? context.game().getGuardianTowerAttackRange()
            : context.game().getFactionBaseAttackRange();
    debug_.circle(unit.getX(), unit.getY(), unit.getVisionRange(), 0x44FF44);
    debug_.circle(unit.getX(), unit.getY(), attack_range, 0xFF4444);
    debug_.text(unit.getX() + unit.getRadius(), unit.getY() + unit.getRadius(),
                std::to_string(unit.getRemainingActionCooldownTicks()).c_str(), 0x444444);

    visualize_unit_mean_life_change_speed(context, unit);

    if (unit.getFaction() == context.self().getFaction()) {
        return;
    }

    const GetTargetScore get_target_score {context};
    const auto score = get_target_score(unit);
    debug_.text(unit.getX() + unit.getRadius(), unit.getY() - unit.getRadius(),
                std::to_string(score).c_str(), get_color(score / (max_target_score ? max_target_score : 1.0)));
}

void DebugStrategy::visualize_unit(const Context& context, const model::Minion& unit) {
    double attack_range = unit.getType() == model::MINION_ORC_WOODCUTTER
            ? context.game().getOrcWoodcutterAttackRange()
            : context.game().getFetishBlowdartAttackRange();
    debug_.circle(unit.getX(), unit.getY(), unit.getVisionRange(), 0x44FF44);
    debug_.circle(unit.getX(), unit.getY(), attack_range, 0xFF4444);
    debug_.text(unit.getX() + unit.getRadius(), unit.getY() + unit.getRadius(),
                std::to_string(unit.getRemainingActionCooldownTicks()).c_str(), 0x444444);

    visualize_unit_speed(unit);
    visualize_unit_mean_speed(context, unit);
    visualize_unit_mean_life_change_speed(context, unit);

    if (unit.getFaction() == context.self().getFaction()) {
        return;
    }

    const GetTargetScore get_target_score {context};
    const auto score = get_target_score(unit);
    const auto interval = max_target_score - min_target_score;
    debug_.text(unit.getX() + unit.getRadius(), unit.getY() - unit.getRadius(),
                std::to_string(score).c_str(), get_color((score - min_target_score) / (interval ? interval : 1.0)));
}

void DebugStrategy::visualize_unit(const Context& context, const model::Tree& unit) {
    if (get_position(context.self()).distance(get_position(unit)) > 200) {
        return;
    }
    const GetTargetScore get_target_score {context};
    const auto score = get_target_score(unit);
    const auto interval = max_target_score - min_target_score;
    debug_.text(unit.getX() + unit.getRadius(), unit.getY() - unit.getRadius(),
                std::to_string(score).c_str(), get_color((score - min_target_score) / (interval ? interval : 1.0)));

    visualize_unit_mean_life_change_speed(context, unit);
}

void DebugStrategy::visualize_unit(const Context& context, const model::Projectile& unit) {
    visualize_unit_speed(unit);
    visualize_unit_mean_speed(context, unit);
}

void DebugStrategy::visualize_states(const Context& context) {
    const auto& states = base_.states();

    if (states.empty()) {
        return;
    }

    const auto& first = states.front();
    const auto& last = states.back();
    const double norm = last.tick() - first.tick();

    for (const auto state : states) {
        const auto heat = double(state.tick() - first.tick()) / (norm ? norm : 1);
        const auto color = get_color(heat);

        debug_.circle(state.position().x(), state.position().y(), context.self().getRadius(), color);

        const auto direction = Point(context.self().getRadius(), 0).rotated(state.angle());
        const auto direction_end = state.position() + direction;

        debug_.line(state.position().x(), state.position().y(), direction_end.x(), direction_end.y(), color);
    }
}

void DebugStrategy::visualize_ticks_states(const Context&) {
    const auto& steps_states = base_.steps_states();

    if (steps_states.empty()) {
        return;
    }

    const auto& ticks_states = base_.ticks_states();

    if (ticks_states.empty()) {
        return;
    }

    const auto& first_tick = steps_states.front().tick();
    const auto& last_tick = steps_states.back().tick();
    const auto norm = last_tick - first_tick;

    for (const auto& step_state : steps_states) {
        const auto ticks_state = ticks_states.find(std::round(step_state.tick()));

        if (ticks_state == ticks_states.end()) {
            continue;
        }

        const auto heat = (step_state.tick() - first_tick) / (norm ? norm : 1);
        const auto color = get_color(heat);

        for (const auto dynamic_barrier : ticks_state->second.dynamic_barriers()) {
            const auto& circle = dynamic_barrier.circle;
            const auto& next_position = dynamic_barrier.target;
            debug_.circle(next_position.x(), next_position.y(), circle.radius(), color);
            debug_.line(circle.position().x(), circle.position().y(), next_position.x(), next_position.y(), color);
        }
    }
}

std::int32_t get_color(double red, double green, double blue) {
    const auto int_red = std::int32_t(std::round(0xFF * red)) << 16;
    const auto int_green = std::int32_t(std::round(0xFF * green)) << 8;
    const auto int_blue = std::int32_t(std::round(0xFF * blue));
    return int_red | int_green | int_blue;
}

std::int32_t get_color(double heat) {
    if (heat < 0.25) {
        return get_color(0, 4 * heat, 1);
    } else if (heat < 0.5) {
        return get_color(0, 1, 1 - 4 * (heat - 0.5));
    } else if (heat < 0.75) {
        return get_color(4 * (heat - 0.5), 1, 0);
    } else {
        return get_color(1, 1 - 4 * (heat - 0.75), 0);
    }
}

}
