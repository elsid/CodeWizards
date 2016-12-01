#include "debug_strategy.hpp"
#include "battle_mode.hpp"
#include "optimal_destination.hpp"
#include "helpers.hpp"

namespace strategy {

static const std::unordered_map<model::ActionType, int> ACTIONS_COLORS = {
    {model::ACTION_STAFF, 0x00FF00},
    {model::ACTION_MAGIC_MISSILE, 0xFF00FF},
    {model::ACTION_FROST_BOLT, 0x4444FF},
    {model::ACTION_FIREBALL, 0xFF0000},
    {model::ACTION_HASTE, 0xFFFF00},
    {model::ACTION_SHIELD, 0x0000FF},
};

static const std::unordered_map<model::ActionType, model::SkillType> ACTIONS_SKILLS = {
    {model::ACTION_STAFF, model::_SKILL_UNKNOWN_},
    {model::ACTION_MAGIC_MISSILE, model::_SKILL_UNKNOWN_},
    {model::ACTION_FROST_BOLT, model::SKILL_FROST_BOLT},
    {model::ACTION_FIREBALL, model::SKILL_FIREBALL},
    {model::ACTION_HASTE, model::SKILL_HASTE},
    {model::ACTION_SHIELD, model::SKILL_SHIELD},
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
    base_->apply(context);
    visualize(context);
}

void DebugStrategy::visualize(const Context& context) {
    Post post(debug_);
    visualize_graph(context);
    visualize_graph_path(context);
    visualize_positions_penalties(context);
    visualize_path(context);
    visualize_destination(context);
    visualize_target(context);
    visualize_units(context);
}

void DebugStrategy::visualize_graph(const Context& context) {
    const GetNodePenalty get_node_penalty(context, base_->graph(), base_->move_mode().target_lane());
    const auto& nodes = base_->graph().nodes();
    const auto& arcs = base_->graph().arcs();
    for (const auto& src : nodes) {
        for (const auto& dst : nodes) {
            if (arcs.get(src.first, dst.first) != std::numeric_limits<double>::max()) {
                debug_.line(src.second.x(), src.second.y(), dst.second.x(), dst.second.y(), 0xAAAAAA);
            }
        }
    }
    for (const auto& node : nodes) {
        const auto penalty = get_node_penalty(node.first);
        debug_.fillCircle(node.second.x(), node.second.y(), 10, get_color(penalty));
        debug_.text(node.second.x() + 30, node.second.y() + 30, std::to_string(node.first).c_str(), 0xAAAAAA);
    }
    const auto friend_base = nodes.at(base_->graph().friend_base());
    debug_.circle(friend_base.x(), friend_base.y(), 40, 0x00AA00);
    const auto enemy_base = nodes.at(base_->graph().enemy_base());
    debug_.circle(enemy_base.x(), enemy_base.y(), 40, 0xAA0000);
}

void DebugStrategy::visualize_graph_path(const Context& context) {
    Point prev = get_position(context.self());
    const auto& nodes = base_->graph().nodes();
    for (auto node = base_->move_mode().path_node(); node != base_->move_mode().path().end(); ++node) {
        const auto position = nodes.at(*node);
        debug_.line(prev.x(), prev.y(), position.x(), position.y(), 0x222222);
        debug_.circle(position.x(), position.y(), 20, 0x222222);
        prev = position;
    }
    const auto& destination = base_->move_mode().destination();
    if (destination.first) {
        const auto& position = nodes.at(destination.second);
        debug_.circle(position.x(), position.y(), 30, 0x222222);
    }
}

void DebugStrategy::visualize_path(const Context& context) {
    auto prev = get_position(context.self());
    for (const auto& point : base_->path()) {
        debug_.line(prev.x(), prev.y(), point.x(), point.y(), 0x000099);
        debug_.fillCircle(point.x(), point.y(), 5, 0x000099);
        prev = point;
    }
}

void DebugStrategy::visualize_positions_penalties(const Context& context) {
    if (const auto target = base_->target().unit<model::Bonus>(context.cache())) {
        visualize_positions_penalties(context, target);
    } else if (const auto target = base_->target().unit<model::Building>(context.cache())) {
        visualize_positions_penalties(context, target);
    } else if (const auto target = base_->target().unit<model::Minion>(context.cache())) {
        visualize_positions_penalties(context, target);
    } else if (const auto target = base_->target().unit<model::Wizard>(context.cache())) {
        visualize_positions_penalties(context, target);
    } else if (const auto target = base_->target().unit<model::Tree>(context.cache())) {
        visualize_positions_penalties(context, target);
    }
}

void DebugStrategy::visualize_destination(const Context& /*context*/) {
    debug_.line(base_->destination().x() - 35, base_->destination().y() - 35,
                base_->destination().x() + 35, base_->destination().y() + 35, 0x0000FF);
    debug_.line(base_->destination().x() + 35, base_->destination().y() - 35,
                base_->destination().x() - 35, base_->destination().y() + 35, 0x0000FF);
}

void DebugStrategy::visualize_target(const Context& context) {
    if (const auto target = base_->target().circular_unit(context.cache())) {
        debug_.circle(target->getX(), target->getY(), target->getRadius() + 20, 0xAA0000);
    }
}

void DebugStrategy::visualize_units(const Context& context) {
    for (const auto& unit : get_units<model::Wizard>(context.cache())) {
        visualize_unit(context, unit.second.value());
    }
    for (const auto& unit : get_units<model::Building>(context.cache())) {
        visualize_unit(context, unit.second.value());
    }
}

void DebugStrategy::visualize_unit(const Context& context, const model::Wizard& unit) {
    debug_.circle(unit.getX(), unit.getY(), unit.getVisionRange(), 0x44FF44);
    debug_.circle(unit.getX(), unit.getY(), unit.getCastRange(), 0xFF4444);
    debug_.circle(unit.getX(), unit.getY(), context.game().getStaffRange(), 0xFF2222);
    const auto missile_direction = Point(1, 0).rotated(normalize_angle(unit.getAngle() + context.move().getCastAngle()));
    const auto missile_min_target = get_position(unit) + missile_direction * context.move().getMinCastDistance();
    const auto missile_max_target = get_position(unit)
            + missile_direction * std::min(unit.getCastRange(), context.move().getMaxCastDistance());
    debug_.line(missile_min_target.x(), missile_min_target.y(), missile_max_target.x(), missile_max_target.y(), 0xFF4444);
    debug_.circle(missile_min_target.x(), missile_min_target.y(), context.game().getMagicMissileRadius(), 0xFF4444);
    debug_.circle(missile_max_target.x(), missile_max_target.y(), context.game().getMagicMissileRadius(), 0xFF4444);
    debug_.text(unit.getX() + unit.getRadius(), unit.getY() + unit.getRadius(),
                std::to_string(unit.getRemainingActionCooldownTicks()).c_str(), 0x444444);

    for (int action = model::ACTION_STAFF, shift = 0; action < model::_ACTION_COUNT_; ++action) {
        const auto a = model::ActionType(action);
        const auto skill = ACTIONS_SKILLS.at(a);
        if (skill != model::_SKILL_UNKNOWN_ && !has_skill(unit, skill)) {
            continue;
        }
        debug_.text(unit.getX() + unit.getRadius() + 35 * (1 + shift), unit.getY() + unit.getRadius(),
                    std::to_string(unit.getRemainingCooldownTicksByAction()[a]).c_str(), ACTIONS_COLORS.at(a));
        ++shift;
    }

    int shilft = 0;
    for (const auto skill : unit.getSkills()) {
        debug_.text(unit.getX() + unit.getRadius(), unit.getY() - unit.getRadius() - 20 * shilft,
                    SKILLS_NAMES.at(skill).c_str(), 0x444444);
        ++shilft;
    }
}

void DebugStrategy::visualize_unit(const Context& context, const model::Building& unit) {
    double attack_range = unit.getType() == model::BUILDING_GUARDIAN_TOWER
            ? context.game().getGuardianTowerAttackRange()
            : context.game().getFactionBaseAttackRange();
    debug_.circle(unit.getX(), unit.getY(), unit.getVisionRange(), 0x44FF44);
    debug_.circle(unit.getX(), unit.getY(), attack_range, 0xFF4444);
    debug_.text(unit.getX() + unit.getRadius(), unit.getY() + unit.getRadius(),
                std::to_string(unit.getRemainingActionCooldownTicks()).c_str(), 0x444444);
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
