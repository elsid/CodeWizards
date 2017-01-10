#pragma once

#include "context.hpp"

#include <map>

namespace strategy {

enum class Specialization {
    FROST,
    RANGE,
    FIRE,
    SHIELD,
    HASTE,
};

model::SkillType get_skill_to_learn(const Context& context, model::SkillType skill_from_message);
model::SkillType get_skill_to_recommend(const model::Wizard& wizard, Specialization specialization);
std::map<UnitId, Specialization> distribute_specializations(const Context& context);

}
