#pragma once

#include "retreat_mode.hpp"
#include "world_graph.hpp"
#include "optimal_movement.hpp"

#include <memory>

namespace strategy {

class Strategy {
public:
    virtual ~Strategy() = default;
    virtual void apply(Context& context) = 0;
};

class BaseStrategy : public Strategy {
public:
    BaseStrategy(const Context& context);

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
    const WorldGraph graph_;
    const std::shared_ptr<BattleMode> battle_mode_;
    const std::shared_ptr<MoveMode> move_mode_;
    const std::shared_ptr<RetreatMode> retreat_mode_;
    std::shared_ptr<Mode> mode_;
    Target target_;
    Point destination_;
    Path path_;
    MovementsStates states_;
    Movements movements_;
    MovementsStates::const_iterator state_;
    Movements::const_iterator movement_;
    model::SkillType skill_from_message_ = model::_SKILL_UNKNOWN_;
    Tick mode_ticks_ = 0;

    void handle_messages(const Context& context);
    void select_mode(const Context& context);
    void apply_mode(const Context& context);
    void update_movements(const Context& context);
    void apply_move(Context& context);
    void apply_action(Context& context);
    void learn_skills(Context& Context);
    void command(Context& context);

    void use_move_mode();
    void use_battle_mode();
    void use_retreat_mode();
    void use_mode(const std::shared_ptr<Mode>& mode);

    void calculate_movements(const Context& context);
};

}
