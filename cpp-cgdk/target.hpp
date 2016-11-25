#pragma once

#include "context.hpp"

namespace strategy {

struct Target {
    const model::Building* building;
    const model::Minion* minion;
    const model::Wizard* wizard;
    const model::Tree* tree;
};

Target get_target(const Context& context);

}
