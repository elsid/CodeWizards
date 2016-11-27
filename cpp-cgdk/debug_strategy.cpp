#include "debug_strategy.hpp"
#include "battle_mode.hpp"

namespace strategy {

struct Pre {
    Debug& debug;

    Pre(Debug& debug) : debug(debug) {
        debug.beginPre();
    }

    ~Pre() {
        debug.endPre();
    }
};

struct Post {
    Debug& debug;

    Post(Debug& debug) : debug(debug) {
        debug.beginPost();
    }

    ~Post() {
        debug.endPost();
    }
};

struct Abs {
    Debug& debug;

    Abs(Debug& debug) : debug(debug) {
        debug.beginAbs();
    }

    ~Abs() {
        debug.endAbs();
    }
};

void DebugStrategy::apply(Context& context) {
    base_->apply(context);
    visualize(context);
}

void DebugStrategy::visualize(const Context& context) {
    Post post(debug_);
    visualize_graph(context);
    visualize_graph_path(context);
    visualize_positions_penalties(context);
    visualize_path(context);
    visualize_destination(context);
    visualize_target(context);
}

void DebugStrategy::visualize_graph(const Context& /*context*/) {
    const auto& nodes = base_->graph().nodes();
    for (const auto& node : nodes) {
        debug_.fillCircle(node.second.x(), node.second.y(), 10, 0xAAAAAA);
        debug_.text(node.second.x() + 30, node.second.y() + 30, std::to_string(node.first).c_str(), 0xAAAAAA);
    }
    const auto& arcs = base_->graph().arcs();
    for (const auto& src : nodes) {
        for (const auto& dst : nodes) {
            if (arcs.get(src.first, dst.first) != std::numeric_limits<double>::max()) {
                debug_.line(src.second.x(), src.second.y(), dst.second.x(), dst.second.y(), 0xAAAAAA);
            }
        }
    }
    const auto friend_base = nodes.at(base_->graph().friend_base());
    debug_.fillCircle(friend_base.x(), friend_base.y(), 20, 0x00AA00);
    const auto enemy_base = nodes.at(base_->graph().enemy_base());
    debug_.fillCircle(enemy_base.x(), enemy_base.y(), 20, 0xAA0000);
}

void DebugStrategy::visualize_graph_path(const Context& context) {
    Point prev = get_position(context.self());
    const auto& nodes = base_->graph().nodes();
    for (auto node = base_->move_mode().path_node(); node != base_->move_mode().path().end(); ++node) {
        const auto position = nodes.at(*node);
        debug_.line(prev.x(), prev.y(), position.x(), position.y(), 0x222222);
        debug_.fillCircle(position.x(), position.y(), 10, 0x222222);
        prev = position;
    }
    const auto& destination = base_->move_mode().destination();
    if (destination.first) {
        const auto& position = nodes.at(destination.second);
        debug_.circle(position.x(), position.y(), 30, 0x222222);
    }
}

void DebugStrategy::visualize_path(const Context& context) {
    auto prev = get_position(context.self());
    for (const auto& point : base_->path()) {
        debug_.line(prev.x(), prev.y(), point.x(), point.y(), 0x000099);
        debug_.fillCircle(point.x(), point.y(), 5, 0x000099);
        prev = point;
    }
}

void DebugStrategy::visualize_positions_penalties(const Context& context) {
    if (const auto target = base_->target().unit<model::Bonus>(context.cache())) {
        visualize_positions_penalties(context, target);
    } else if (const auto target = base_->target().unit<model::Building>(context.cache())) {
        visualize_positions_penalties(context, target);
    } else if (const auto target = base_->target().unit<model::Minion>(context.cache())) {
        visualize_positions_penalties(context, target);
    } else if (const auto target = base_->target().unit<model::Wizard>(context.cache())) {
        visualize_positions_penalties(context, target);
    } else if (const auto target = base_->target().unit<model::Tree>(context.cache())) {
        visualize_positions_penalties(context, target);
    }
}

void DebugStrategy::visualize_destination(const Context& /*context*/) {
    debug_.line(base_->destination().x() - 35, base_->destination().y() - 35,
                base_->destination().x() + 35, base_->destination().y() + 35, 0x0000FF);
    debug_.line(base_->destination().x() + 35, base_->destination().y() - 35,
                base_->destination().x() - 35, base_->destination().y() + 35, 0x0000FF);
}

void DebugStrategy::visualize_target(const Context& context) {
    if (const auto target = base_->target().circular_unit(context.cache())) {
        debug_.circle(target->getX(), target->getY(), target->getRadius() + 20, 0xFF00000);
    }
}

std::int32_t get_color(double red, double green, double blue) {
    const auto int_red = std::int32_t(std::round(0xFF * red)) << 16;
    const auto int_green = std::int32_t(std::round(0xFF * green)) << 8;
    const auto int_blue = std::int32_t(std::round(0xFF * blue));
    return int_red | int_green | int_blue;
}

std::int32_t get_color(double heat) {
    if (heat < 0.25) {
        return get_color(0, 4 * heat, 1);
    } else if (heat < 0.5) {
        return get_color(0, 1, 1 - 4 * (heat - 0.5));
    } else if (heat < 0.75) {
        return get_color(4 * (heat - 0.5), 1, 0);
    } else {
        return get_color(1, 1 - 4 * (heat - 0.75), 0);
    }
}

}
