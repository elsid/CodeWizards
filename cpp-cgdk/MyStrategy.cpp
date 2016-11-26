#include "MyStrategy.h"

#include "cache.hpp"

#ifndef STRATEGY_DEBUG

#include "time_limited_strategy.hpp"

#endif

#include "debug/output.hpp"

#include <iostream>

void MyStrategy::move(const model::Wizard& self, const model::World& world, const model::Game& game, model::Move& move) {
    try {
        strategy::Profiler profiler;
        strategy::FullCache cache;
        strategy::Context context(self, world, game, move, cache, profiler, strategy::Duration::max());
        if (!strategy_) {
            auto base = std::make_unique<strategy::Strategy>(context);
#ifdef STRATEGY_DEBUG
            strategy_ = std::move(base);
#else
            strategy_ = std::make_unique<strategy::TimeLimitedStrategy>(std::move(base));
#endif
        }
        strategy_->apply(context);
    } catch (const std::exception& exception) {
#ifdef STRATEGY_DEBUG
        std::cerr << exception.what() << std::endl;
        throw;
#else
        strategy_.reset();
#endif
    }
}
