#pragma once

#include "strategy.hpp"

#include "russian-ai-cup-visual/Debug.h"

namespace strategy {

class DebugStrategy : public IStrategy {
public:
    DebugStrategy(std::unique_ptr<Strategy> base) : base_(std::move(base)) {}

    void apply(Context& context) override final;

private:
    std::unique_ptr<Strategy> base_;
    Debug debug_;

    void visualize(const Context& context);
    void visualize_graph(const Context& context);
    void visualize_graph_path(const Context& context);
    void visualize_points(const Context& context);
    void visualize_path(const Context& context);
    void visualize_destination(const Context& context);
    void visualize_states(const Context& context);
    void visualize_target(const Context& context);
};

}
