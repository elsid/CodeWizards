#include "optimal_movement.hpp"
#include "optimal_target.hpp"
#include "optimal_position.hpp"
#include "optimal_path.hpp"

namespace strategy {

struct Bounds {
    const Context& context;
};

std::pair<States, Movements> get_optimal_movement(const Context& context, const Point& target, double step_size) {
    get_optimal_path(context, target, step_size);

    const Bounds bounds {context};

    return std::pair<States, Movements>();
}

}
