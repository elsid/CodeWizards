#include "skills.hpp"
#include "helpers.hpp"

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

const std::array<int, model::_SKILL_COUNT_> FROST_SKILLS_PRIORITIES = {{
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

const std::array<int, model::_SKILL_COUNT_> RANGE_SKILLS_PRIORITIES = {{
    5, // SKILL_RANGE_BONUS_PASSIVE_1
    5, // SKILL_RANGE_BONUS_AURA_1
    5, // SKILL_RANGE_BONUS_PASSIVE_2
    5, // SKILL_RANGE_BONUS_AURA_2
    5, // SKILL_ADVANCED_MAGIC_MISSILE
    4, // SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_1
    4, // SKILL_MAGICAL_DAMAGE_BONUS_AURA_1
    4, // SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_2
    4, // SKILL_MAGICAL_DAMAGE_BONUS_AURA_2
    4, // SKILL_FROST_BOLT
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

const std::array<int, model::_SKILL_COUNT_> FIRE_SKILLS_PRIORITIES = {{
    3, // SKILL_RANGE_BONUS_PASSIVE_1
    3, // SKILL_RANGE_BONUS_AURA_1
    3, // SKILL_RANGE_BONUS_PASSIVE_2
    3, // SKILL_RANGE_BONUS_AURA_2
    3, // SKILL_ADVANCED_MAGIC_MISSILE
    4, // SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_1
    4, // SKILL_MAGICAL_DAMAGE_BONUS_AURA_1
    4, // SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_2
    4, // SKILL_MAGICAL_DAMAGE_BONUS_AURA_2
    4, // SKILL_FROST_BOLT
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

const std::array<int, model::_SKILL_COUNT_> SHIELD_SKILLS_PRIORITIES = {{
    3, // SKILL_RANGE_BONUS_PASSIVE_1
    3, // SKILL_RANGE_BONUS_AURA_1
    3, // SKILL_RANGE_BONUS_PASSIVE_2
    3, // SKILL_RANGE_BONUS_AURA_2
    3, // SKILL_ADVANCED_MAGIC_MISSILE
    4, // SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_1
    4, // SKILL_MAGICAL_DAMAGE_BONUS_AURA_1
    4, // SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_2
    4, // SKILL_MAGICAL_DAMAGE_BONUS_AURA_2
    4, // SKILL_FROST_BOLT
    2, // SKILL_STAFF_DAMAGE_BONUS_PASSIVE_1
    2, // SKILL_STAFF_DAMAGE_BONUS_AURA_1
    2, // SKILL_STAFF_DAMAGE_BONUS_PASSIVE_2
    2, // SKILL_STAFF_DAMAGE_BONUS_AURA_2
    2, // SKILL_FIREBALL
    1, // SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_1
    1, // SKILL_MOVEMENT_BONUS_FACTOR_AURA_1
    1, // SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_2
    1, // SKILL_MOVEMENT_BONUS_FACTOR_AURA_2
    1, // SKILL_HASTE
    5, // SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_1
    5, // SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_1
    5, // SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_2
    5, // SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_2
    5, // SKILL_SHIELD
}};

const std::array<int, model::_SKILL_COUNT_> HASTE_SKILLS_PRIORITIES = {{
    3, // SKILL_RANGE_BONUS_PASSIVE_1
    3, // SKILL_RANGE_BONUS_AURA_1
    3, // SKILL_RANGE_BONUS_PASSIVE_2
    3, // SKILL_RANGE_BONUS_AURA_2
    3, // SKILL_ADVANCED_MAGIC_MISSILE
    4, // SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_1
    4, // SKILL_MAGICAL_DAMAGE_BONUS_AURA_1
    4, // SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_2
    4, // SKILL_MAGICAL_DAMAGE_BONUS_AURA_2
    4, // SKILL_FROST_BOLT
    2, // SKILL_STAFF_DAMAGE_BONUS_PASSIVE_1
    2, // SKILL_STAFF_DAMAGE_BONUS_AURA_1
    2, // SKILL_STAFF_DAMAGE_BONUS_PASSIVE_2
    2, // SKILL_STAFF_DAMAGE_BONUS_AURA_2
    2, // SKILL_FIREBALL
    5, // SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_1
    5, // SKILL_MOVEMENT_BONUS_FACTOR_AURA_1
    5, // SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_2
    5, // SKILL_MOVEMENT_BONUS_FACTOR_AURA_2
    5, // SKILL_HASTE
    1, // SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_1
    1, // SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_1
    1, // SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_2
    1, // SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_2
    1, // SKILL_SHIELD
}};

const std::array<int, model::_SKILL_COUNT_>& DEFAULT_SKILLS_PRIORITIES = FIRE_SKILLS_PRIORITIES;

std::array<const std::array<int, model::_SKILL_COUNT_>, model::_SKILL_COUNT_> SPECIALIZATIONS = {{
    FROST_SKILLS_PRIORITIES,
    RANGE_SKILLS_PRIORITIES,
    FIRE_SKILLS_PRIORITIES,
    SHIELD_SKILLS_PRIORITIES,
    HASTE_SKILLS_PRIORITIES,
}};

std::pair<model::SkillType, int> get_opposite_skill(const Context& context) {
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

model::SkillType get_next_skill_to_learn(const model::Wizard& unit, model::SkillType skill) {
    if (has_skill(unit, skill)) {
        return model::_SKILL_UNKNOWN_;
    }
    while (true) {
        const auto prev = SKILLS_DEPENDECIES.at(skill);
        if (prev == model::_SKILL_UNKNOWN_ || has_skill(unit, prev)) {
            return skill;
        }
        skill = prev;
    }
}

model::SkillType get_skill_to_learn(const Context& context, model::SkillType skill_from_message) {
    auto skills_priorities = DEFAULT_SKILLS_PRIORITIES;

    for (const auto skill : context.self().getSkills()) {
        skills_priorities[skill] = 0;
    }

    if (skill_from_message != model::_SKILL_UNKNOWN_ && skill_from_message != model::_SKILL_COUNT_
            && !has_skill(context.self(), skill_from_message)) {
        skills_priorities[skill_from_message] += 10;
    }

    if (!context.game().isRawMessagesEnabled()) {
        model::SkillType opposite;
        int opposite_priority;
        std::tie(opposite, opposite_priority) = get_opposite_skill(context);

        if (opposite != model::_SKILL_UNKNOWN_) {
            skills_priorities[opposite] += opposite_priority;
        }
    }

    const auto max = std::max_element(skills_priorities.begin(), skills_priorities.end());
    const auto skill_to_learn = model::SkillType(max - skills_priorities.begin());

    return get_next_skill_to_learn(context.self(), skill_to_learn);
}

model::SkillType get_skill_to_recommend(const model::Wizard& wizard, Specialization specialization) {
    auto skills_priorities = SPECIALIZATIONS[std::size_t(specialization)];

    for (const auto skill : wizard.getSkills()) {
        skills_priorities[skill] = 0;
    }

    const auto max = std::max_element(skills_priorities.begin(), skills_priorities.end());

    return get_next_skill_to_learn(wizard, model::SkillType(max - skills_priorities.begin()));
}

const std::array<Specialization, 5> FIRE_FROST_SHIELD_SPECIALIZATIONS_DISTRIBUTION = {{
    Specialization::SHIELD, // master
    Specialization::FIRE,
    Specialization::FROST,
    Specialization::FIRE,
    Specialization::FIRE,
}};

const std::array<Specialization, 5> FIRE_FROST_SPECIALIZATIONS_DISTRIBUTION = {{
    Specialization::FIRE, // master
    Specialization::FIRE,
    Specialization::FROST,
    Specialization::FIRE,
    Specialization::FIRE,
}};

const std::array<Specialization, 5>& get_specializations_distribution(const Context& context) {
    if (context.game().isRawMessagesEnabled()) {
        return FIRE_FROST_SHIELD_SPECIALIZATIONS_DISTRIBUTION;
    } else {
        return FIRE_FROST_SPECIALIZATIONS_DISTRIBUTION;
    }
}

std::map<UnitId, Specialization> distribute_specializations(const Context& context) {
    const auto& distribution = get_specializations_distribution(context);
    auto specialization = distribution.begin();
    std::map<UnitId, Specialization> result;
    for (const auto& v : get_units<model::Wizard>(context.cache())) {
        const auto& unit = v.second;
        if (unit.value().getFaction() == context.self().getFaction()) {
            result.insert({v.first, *specialization++});
            if (specialization == distribution.end()) {
                specialization = distribution.begin();
            }
        }
    }
    return result;
}

}
