#pragma once

#ifndef _MY_STRATEGY_H_
#define _MY_STRATEGY_H_

#include "Strategy.h"
#include "time_limited_strategy.hpp"

class MyStrategy : public Strategy {
public:
    void move(const model::Wizard& self, const model::World& world, const model::Game& game, model::Move& move) override;

private:
    strategy::FullCache cache_;
    strategy::FullCache history_cache_;
    std::unique_ptr<strategy::Strategy> strategy_;

#ifdef ELSID_STRATEGY_DEBUG
    void debug(strategy::Context& context);
#endif

    void release(strategy::Context& context);
    void update_cache(const model::Wizard& self, const model::World& world);
    void add_fake_bonuses(const model::World& world);
    void add_fake_enemy_buildings(const model::World& world, model::Faction enemy_faction);
};

#endif
