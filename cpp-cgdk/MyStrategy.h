#pragma once

#ifndef _MY_STRATEGY_H_
#define _MY_STRATEGY_H_

#include "Strategy.h"
#include "strategy.hpp"

class MyStrategy : public Strategy {
public:
    void move(const model::Wizard& self, const model::World& world, const model::Game& game, model::Move& move) override;

private:
    strategy::Strategy strategy_;
};

#endif
