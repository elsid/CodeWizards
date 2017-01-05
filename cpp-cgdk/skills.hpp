#pragma once

#include "context.hpp"

namespace strategy {

model::SkillType get_skill_to_learn(const Context& context, model::SkillType skill_from_message);

}
