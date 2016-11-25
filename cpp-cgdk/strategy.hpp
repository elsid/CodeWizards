#pragma once

#include "target.hpp"

namespace strategy {

class Strategy {
public:
    void apply(Context& context) {
        const auto target = get_target(context);
        get_optimal_position(context, target);
    }
};

}
