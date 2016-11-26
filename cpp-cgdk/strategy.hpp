#pragma once

#include "optimal_position.hpp"
#include "world_graph.hpp"

#include <memory>

#ifdef STRATEGY_DEBUG

#include "debug/output.hpp"

#include <iostream>

#endif

namespace strategy {

class IStrategy {
public:
    virtual ~IStrategy() = default;
    virtual void apply(Context& context) = 0;
};

class Strategy : public IStrategy {
public:
    Strategy(const Context& context);

    void apply(Context& context) override final;

private:
    WorldGraph graph_;
};

}
