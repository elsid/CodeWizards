#pragma once

#include "target.hpp"
#include "optimal_position.hpp"

#ifdef STRATEGY_DEBUG

#include "debug/output.hpp"

#include <iostream>

#endif

namespace strategy {

class Strategy {
public:
    void apply(Context& context) {
//        std::cout << context.self << std::endl;
        std::cout << context.world << std::endl;
//        std::cout << context.game << std::endl;
//        const auto target = get_target(context);
//        get_optimal_position(context, target);
    }
};

}
