#pragma once

#include "optimal_position.hpp"
#include "world_graph.hpp"

#include <memory>

#ifdef STRATEGY_DEBUG

#include "debug/output.hpp"

#include <iostream>

#endif

namespace strategy {

class Strategy {
public:
    void apply(Context& context);

private:
    bool initialized_ = false;
    std::unique_ptr<WorldGraph> graph_;

    void initialize(const Context& context);
};

}
