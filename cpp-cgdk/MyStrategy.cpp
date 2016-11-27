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
        update_cache(self, world);
        strategy::Context context(self, world, game, move, cache_, profiler, strategy::Duration::max());
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

namespace strategy {

template <class T>
struct CacheTtl {};

template <>
struct CacheTtl<model::Bonus> {
    static constexpr const Tick value = 2500;
};

template <>
struct CacheTtl<model::Building> {
    static constexpr const Tick value = 2500;
};

template <>
struct CacheTtl<model::Minion> {
    static constexpr const Tick value = 30;
};

template <>
struct CacheTtl<model::Projectile> {
    static constexpr const Tick value = 10;
};

template <>
struct CacheTtl<model::Tree> {
    static constexpr const Tick value = 2500;
};

template <>
struct CacheTtl<model::Wizard> {
    static constexpr const Tick value = 30;
};

}

void MyStrategy::update_cache(const model::Wizard& self, const model::World& world) {
    using namespace strategy;

    strategy::update_cache(cache_, world);

    const auto is_friend_or_me = [&] (const auto& unit) {
        return unit.getFaction() == self.getFaction();
    };

    const auto buildings = filter_units(get_units<model::Building>(world), is_friend_or_me);
    const auto minions = filter_units(get_units<model::Minion>(world), is_friend_or_me);
    const auto wizards = filter_units(get_units<model::Wizard>(world), is_friend_or_me);

    const auto need_invalidate = [&] (const auto& unit) {
        using Type = typename std::decay<decltype(unit.value())>::type;

        if (world.getTickIndex() - unit.last_seen() > CacheTtl<Type>::value) {
            return true;
        }

        const auto is_in_range = [&] (auto other) {
            return unit.last_seen() < world.getTickIndex()
                    && get_position(*other).distance(get_position(unit.value()))
                    <= other->getVisionRange() - 2 * unit.value().getRadius();
        };

        return buildings.end() != std::find_if(buildings.begin(), buildings.end(), is_in_range)
                || minions.end() != std::find_if(minions.begin(), minions.end(), is_in_range)
                || wizards.end() != std::find_if(wizards.begin(), wizards.end(), is_in_range);
    };

    invalidate_cache(cache_, need_invalidate);
}
