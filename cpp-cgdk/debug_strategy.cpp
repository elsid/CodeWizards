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

//    std::cout << " tick: " << context.world().getTickIndex()
//              << " destination: " << base_->destination()
//              << " position: " << get_position(context.self()) << std::endl;

    visualize(context);
}

void DebugStrategy::visualize(const Context& context) {
    Post post(debug_);
    visualize_graph(context);
    visualize_graph_path(context);
    visualize_points(context);
    visualize_path(context);
    visualize_destination(context);
//    visualize_states(context);
    visualize_target(context);
}

void DebugStrategy::visualize_graph(const Context& context) {
    for (const auto& node : base_->graph().nodes()) {
        debug_.fillCircle(node.second.x(), node.second.y(), 10, 0xAAAAAA);
    }
    const auto& arcs = base_->graph().arcs();
    for (const auto& src : base_->graph().nodes()) {
        for (const auto& dst : base_->graph().nodes()) {
            if (arcs.get(src.first, dst.first) != std::numeric_limits<double>::max()) {
                debug_.line(src.second.x(), src.second.y(), dst.second.x(), dst.second.y(), 0xAAAAAA);
            }
        }
    }
}

void DebugStrategy::visualize_graph_path(const Context& context) {
    Point prev = get_position(context.self());
    for (auto node = base_->move_mode().path_node(); node != base_->move_mode().path().end(); ++node) {
        const auto position = base_->graph().nodes().at(*node);
        debug_.line(prev.x(), prev.y(), position.x(), position.y(), 0x222222);
        debug_.fillCircle(position.x(), position.y(), 10, 0x222222);
        prev = position;
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

void DebugStrategy::visualize_points(const Context& context) {
    const auto& battle_mode = base_->battle_mode();
    if (battle_mode.points().empty()) {
        return;
    }
    const auto min_max = std::minmax_element(battle_mode.points().begin(), battle_mode.points().end(),
        [] (const auto& lhs, const auto& rhs) { return lhs.second < rhs.second; });
    const auto min = min_max.first->second;
    const auto max = min_max.second->second;
    const auto norm = max != min ? std::abs(max - min) : 1;
    for (const auto& point : battle_mode.points()) {
        const auto normalized = (point.second - min) / norm;
        debug_.fillCircle(base_->destination().x(), base_->destination().y(), 7, get_color(normalized));
    }
}

void DebugStrategy::visualize_destination(const Context& context) {
    debug_.line(base_->destination().x() - 50, base_->destination().y() - 50,
                base_->destination().x() + 50, base_->destination().y() + 50, 0x0000FF);
    debug_.line(base_->destination().x() + 50, base_->destination().y() - 50,
                base_->destination().x() - 50, base_->destination().y() + 50, 0x0000FF);
}

void DebugStrategy::visualize_states(const Context& context) {
    auto prev = get_position(context.self());
    for (const auto& state : base_->states()) {
        debug_.line(prev.x(), prev.y(), state.position().x(), state.position().y(), 0x009900);
        prev = state.position();
    }
}

void DebugStrategy::visualize_target(const Context& context) {
    if (const auto target = base_->target().circular_unit(base_->cache())) {
        debug_.circle(target->getX(), target->getY(), target->getRadius() + 20, 0xFF00000);
    }
}

}
