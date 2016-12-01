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
    std::unique_ptr<strategy::Strategy> strategy_;

    void update_cache(const model::Wizard& self, const model::World& world);
    void add_fake_bonuses(const model::World& world);
    void add_fake_enemy_buildings(const model::World& world, model::Faction enemy_faction);
};

#endif
