#include "MyStrategy.h"

#include "optimal_target.hpp"
#include "cache.hpp"

#ifdef STRATEGY_DEBUG

#include "debug_strategy.hpp"

#include "debug/output.hpp"

#else

#include "time_limited_strategy.hpp"

#endif

#ifdef STRATEGY_LOCAL

#include <iostream>

#endif

void MyStrategy::move(const model::Wizard& self, const model::World& world, const model::Game& game, model::Move& move) {
#ifndef STRATEGY_DEBUG
    try {
#endif
        strategy::Profiler profiler;
        add_fake_bonuses(world);
        add_fake_enemy_buildings(world, self.getFaction() == model::FACTION_ACADEMY ? model::FACTION_RENEGADES : model::FACTION_ACADEMY);
        update_cache(self, world);
        strategy::Context context(self, world, game, move, cache_, profiler, strategy::Duration::max());
        if (!strategy_) {
            auto base = std::make_unique<strategy::BaseStrategy>(context);
#ifdef STRATEGY_DEBUG
            strategy_ = std::make_unique<strategy::DebugStrategy>(std::move(base));
#else
            strategy_ = std::make_unique<strategy::TimeLimitedStrategy>(std::move(base));
#endif
        }
        strategy_->apply(context);
#ifndef STRATEGY_DEBUG
    } catch (const std::exception& exception) {
#ifdef STRATEGY_LOCAL
        std::cerr << "[" << world.getTickIndex() << "] " << exception.what() << std::endl;
#endif
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
    static constexpr const Tick value = std::numeric_limits<int>::max();
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
    static constexpr const Tick value = std::numeric_limits<int>::max();
};

template <>
struct CacheTtl<model::Wizard> {
    static constexpr const Tick value = 30;
};

static const model::Bonus FAKE_TOP_BONUS(
    -1, // Id
    1200, // X
    1200, // Y
    0, // SpeedX
    0, // SpeedY
    0, // Angle
    model::FACTION_OTHER, // Faction
    0, // Radius
    model::BONUS_EMPOWER // Type
);

static const model::Bonus FAKE_BOTTOM_BONUS(
    -2, // Id
    2800, // X
    2800, // Y
    0, // SpeedX
    0, // SpeedY
    0, // Angle
    model::FACTION_OTHER, // Faction
    0, // Radius
    model::BONUS_EMPOWER // Type
);

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
                    <= other->getVisionRange() - unit.value().getRadius();
        };

        return buildings.end() != std::find_if(buildings.begin(), buildings.end(), is_in_range)
                || minions.end() != std::find_if(minions.begin(), minions.end(), is_in_range)
                || wizards.end() != std::find_if(wizards.begin(), wizards.end(), is_in_range);
    };

    invalidate_cache(cache_, need_invalidate);
}

void MyStrategy::add_fake_bonuses(const model::World& world) {
    if (world.getTickIndex() == 0 || world.getTickIndex() % 2500 != 0) {
        return;
    }
    strategy::get_cache<model::Bonus>(cache_).update(strategy::FAKE_TOP_BONUS, world.getTickIndex());
    strategy::get_cache<model::Bonus>(cache_).update(strategy::FAKE_BOTTOM_BONUS, world.getTickIndex());
}

void MyStrategy::add_fake_enemy_buildings(const model::World& world, model::Faction enemy_faction) {
    if (world.getTickIndex() != 1000) {
        return;
    }

    static const std::vector<model::Building> FAKE_ENEMY_BUILDINGS({
        model::Building(
            -3, // Id
            world.getWidth() - 400, // X
            world.getHeight() - 3600, // Y
            0, // SpeedX
            0, // SpeedY
            0, // Angle
            enemy_faction, // Faction
            100, // Radius
            1000, // Life
            1000, // MaxLife
            {}, // Statuses
            model::BUILDING_FACTION_BASE, // Type
            800, // VisionRange
            800, // AttackRange
            0, // Damage
            240, // CooldownTicks
            0 // RemainingActionCooldownTicks
        ),
        model::Building(
            -4, // Id
            world.getWidth() - 50, // X
            world.getHeight() - 2693.26, // Y
            0, // SpeedX
            0, // SpeedY
            0, // Angle
            enemy_faction, // Faction
            50, // Radius
            500, // Life
            500, // MaxLife
            {}, // Statuses
            model::BUILDING_GUARDIAN_TOWER, // Type
            600, // VisionRange
            600, // AttackRange
            0, // Damage
            240, // CooldownTicks
            0 // RemainingActionCooldownTicks
        ),
        model::Building(
            -5, // Id
            world.getWidth() - 350, // X
            world.getHeight() - 1656.75, // Y
            0, // SpeedX
            0, // SpeedY
            0, // Angle
            enemy_faction, // Faction
            50, // Radius
            500, // Life
            500, // MaxLife
            {}, // Statuses
            model::BUILDING_GUARDIAN_TOWER, // Type
            600, // VisionRange
            600, // AttackRange
            0, // Damage
            240, // CooldownTicks
            0 // RemainingActionCooldownTicks
        ),
        model::Building(
            -6, // Id
            world.getWidth() - 902.613, // X
            world.getHeight() - 2768.1, // Y
            0, // SpeedX
            0, // SpeedY
            0, // Angle
            enemy_faction, // Faction
            50, // Radius
            500, // Life
            500, // MaxLife
            {}, // Statuses
            model::BUILDING_GUARDIAN_TOWER, // Type
            600, // VisionRange
            600, // AttackRange
            0, // Damage
            240, // CooldownTicks
            0 // RemainingActionCooldownTicks
        ),
        model::Building(
            -7, // Id
            world.getWidth() - 1370.66, // X
            world.getHeight() - 3650, // Y
            0, // SpeedX
            0, // SpeedY
            0, // Angle
            enemy_faction, // Faction
            50, // Radius
            500, // Life
            500, // MaxLife
            {}, // Statuses
            model::BUILDING_GUARDIAN_TOWER, // Type
            600, // VisionRange
            600, // AttackRange
            0, // Damage
            240, // CooldownTicks
            0 // RemainingActionCooldownTicks
        ),
        model::Building(
            -8, // Id
            world.getWidth() - 1929.29, // X
            world.getHeight() - 2400, // Y
            0, // SpeedX
            0, // SpeedY
            0, // Angle
            enemy_faction, // Faction
            50, // Radius
            500, // Life
            500, // MaxLife
            {}, // Statuses
            model::BUILDING_GUARDIAN_TOWER, // Type
            600, // VisionRange
            600, // AttackRange
            0, // Damage
            240, // CooldownTicks
            0 // RemainingActionCooldownTicks
        ),
        model::Building(
            -9, // Id
            world.getWidth() - 2312.13, // X
            world.getHeight() - 3950, // Y
            0, // SpeedX
            0, // SpeedY
            0, // Angle
            enemy_faction, // Faction
            50, // Radius
            500, // Life
            500, // MaxLife
            {}, // Statuses
            model::BUILDING_GUARDIAN_TOWER, // Type
            600, // VisionRange
            600, // AttackRange
            0, // Damage
            240, // CooldownTicks
            0 // RemainingActionCooldownTicks
        ),
    });

    for (const auto& unit : FAKE_ENEMY_BUILDINGS) {
        strategy::get_cache<model::Building>(cache_).update(unit, world.getTickIndex());
    }
}
