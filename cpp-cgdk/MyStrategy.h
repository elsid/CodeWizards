#pragma once

#ifndef _MY_STRATEGY_H_
#define _MY_STRATEGY_H_

#include "Strategy.h"
#include "time_limited_strategy.hpp"

class MyStrategy : public Strategy {
public:
    void move(const model::Wizard& self, const model::World& world, const model::Game& game, model::Move& move) override;

private:
    std::unique_ptr<strategy::IStrategy> strategy_;
};

#endif
