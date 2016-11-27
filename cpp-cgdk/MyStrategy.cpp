#include "MyStrategy.h"

#include "cache.hpp"

#ifdef STRATEGY_DEBUG

#include "debug_strategy.hpp"

#include "debug/output.hpp"

#include <iostream>

#else

#include "time_limited_strategy.hpp"

#endif

void MyStrategy::move(const model::Wizard& self, const model::World& world, const model::Game& game, model::Move& move) {
#ifndef STRATEGY_DEBUG
    try {
#endif
        strategy::Profiler profiler;
        strategy::FullCache cache;
        strategy::Context context(self, world, game, move, cache, profiler, strategy::Duration::max());
        if (!strategy_) {
            auto base = std::make_unique<strategy::Strategy>(context);
#ifdef STRATEGY_DEBUG
            strategy_ = std::make_unique<strategy::DebugStrategy>(std::move(base));
#else
            strategy_ = std::make_unique<strategy::TimeLimitedStrategy>(std::move(base));
#endif
        }
        strategy_->apply(context);
#ifndef STRATEGY_DEBUG
    } catch (const std::exception& exception) {
        strategy_.reset();
    }
#endif
}
