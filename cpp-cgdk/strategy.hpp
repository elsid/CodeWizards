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
    FullCache cache_;

    void update_cache(const Context& context);

    template <class T>
    void update_specific_cache(const Context& context) {
        std::get<Cache<T>>(cache_).update(get_units<T>(context.world()), context.world().getTickIndex());
    }
};

}
