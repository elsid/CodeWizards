#pragma once

#include "Strategy.h"
#include "base_strategy.hpp"

class MyStrategy : public Strategy {
public:
    void move(const model::Wizard& self, const model::World& world, const model::Game& game, model::Move& move) override;

private:
    strategy::FullCache cache_;
    strategy::FullCache history_cache_;
    std::unique_ptr<strategy::AbstractStrategy> strategy_;

    void update_cache(const model::Wizard& self, const model::World& world);
    void add_fake_bonuses(const model::World& world);
    void add_fake_enemy_buildings(const model::World& world, model::Faction enemy_faction);
};
