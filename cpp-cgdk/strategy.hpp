#pragma once

#include "mode.hpp"
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
    std::unique_ptr<Mode> mode_;
    Target target_;
    Point destination_;
    MovementsStates states_;
    Movements movements_;
    MovementsStates::const_iterator state_;
    Movements::const_iterator movement_;

    void update_cache(const Context& context);
    void handle_messages(const Context& context);
    void apply_mode(const Context& context);
    void update_movements(const Context& context);
    void apply_move(Context& context);
    void apply_action(Context& context);

    void use_move_mode();
    void use_battle_mode();
    void calculate_movements(const Context& context);

    template <class T>
    void update_specific_cache(const Context& context) {
        std::get<Cache<T>>(cache_).update(get_units<T>(context.world()), context.world().getTickIndex());
    }

    static bool need_apply_staff(const Context& context, const model::CircularUnit& target);
    static bool need_apply_magic_missile(const Context& context, const model::CircularUnit& target, double turn);
};

}
