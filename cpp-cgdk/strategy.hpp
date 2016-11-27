#pragma once

#include "battle_mode.hpp"
#include "move_mode.hpp"
#include "optimal_position.hpp"
#include "world_graph.hpp"
#include "optimal_path.hpp"
#include "optimal_movement.hpp"

#include <memory>

namespace strategy {

class IStrategy {
public:
    virtual ~IStrategy() = default;
    virtual void apply(Context& context) = 0;
};

class Strategy : public IStrategy {
public:
    Strategy(const Context& context);

    const WorldGraph& graph() const {
        return graph_;
    }

    const Path& path() const {
        return path_;
    }

    const MovementsStates& states() const {
        return states_;
    }

    const Point& destination() const {
        return destination_;
    }

    const Target& target() const {
        return target_;
    }

    const BattleMode& battle_mode() const {
        return *battle_mode_;
    }

    const MoveMode& move_mode() const {
        return *move_mode_;
    }

    void apply(Context& context) override final;

private:
    WorldGraph graph_;
    const std::shared_ptr<BattleMode> battle_mode_;
    const std::shared_ptr<MoveMode> move_mode_;
    std::shared_ptr<Mode> mode_;
    Target target_;
    Point destination_;
    Path path_;
    MovementsStates states_;
    Movements movements_;
    MovementsStates::const_iterator state_;
    Movements::const_iterator movement_;

    void select_mode(const Context& context);
    void apply_mode(const Context& context);
    void update_movements(const Context& context);
    void apply_move(Context& context);
    void apply_action(Context& context);

    void use_move_mode();
    void use_battle_mode();
    void calculate_movements(const Context& context);

    static bool need_apply_staff(const Context& context, const model::CircularUnit& target);
    static bool need_apply_magic_missile(const Context& context, const model::CircularUnit& target, double turn);
};

}
