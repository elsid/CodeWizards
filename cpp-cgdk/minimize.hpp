#ifndef STRATEGY_MINIMIZE_HPP
#define STRATEGY_MINIMIZE_HPP

#include "newuoa.h"
#include "point.hpp"

namespace strategy {

template <class F>
NewuoaClosureConst make_closure(F &function) {
    struct Wrap {
        static double call(const void *data, long n, const double *values) {
            return reinterpret_cast<const F *>(data)->operator()(n, values);
        }
    };
    return NewuoaClosureConst {&function, &Wrap::call};
}

template <class F>
Point minimize(const F& function, const Point& initial, long max_function_calls_count) {
    const long variables_count = 2;
    const int number_of_interpolation_conditions = variables_count + 2;
//    const long number_of_interpolation_conditions = (variables_count + 1) * (variables_count + 2) / 2;
    double variables_values[] = {initial.x(), initial.y()};
    const double initial_trust_region_radius = 1;
    const double final_trust_region_radius = 1e3;
    const std::size_t working_space_size = NEWUOA_WORKING_SPACE_SIZE(variables_count, number_of_interpolation_conditions);
    double working_space[working_space_size];

    const auto wrapper = [&] (long, const double *x) {
        return function(Point(x[0], x[1]));
    };
    const auto closure = make_closure(wrapper);

    newuoa_closure_const(
        &closure,
        variables_count,
        number_of_interpolation_conditions,
        variables_values,
        initial_trust_region_radius,
        final_trust_region_radius,
        max_function_calls_count,
        working_space
    );

    return Point(variables_values[0], variables_values[1]);
}

}

#endif
