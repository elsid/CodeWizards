#ifndef STRATEGY_MINIMIZE_HPP
#define STRATEGY_MINIMIZE_HPP

#include "newuoa.h"
#include "point.hpp"

namespace strategy {

template <class Function>
NewuoaClosureConst make_closure(const Function& function) {
    struct Wrap {
        static double call(const void* data, long n, const double* values) {
            return reinterpret_cast<const Function*>(data)->operator()(n, values);
        }
    };
    return NewuoaClosureConst {&function, &Wrap::call};
}

class Minimize {
public:
    template <class Function>
    std::pair<double, Point> operator ()(const Point& initial, const Function& function) const {
        double variables_values[] = {initial.x(), initial.y()};
        const auto working_space_size = NEWUOA_WORKING_SPACE_SIZE(variables_count_, number_of_interpolation_conditions_);
        double working_space[working_space_size];

        const auto wrapper = [&] (long, const double *x) {
            return function(Point(x[0], x[1]));
        };
        const auto closure = make_closure(wrapper);

        const auto result = newuoa_closure_const(
            &closure,
            variables_count_,
            number_of_interpolation_conditions_,
            variables_values,
            initial_trust_region_radius_,
            final_trust_region_radius_,
            max_function_calls_count_,
            working_space
        );

        return {result, Point(variables_values[0], variables_values[1])};
    }

    Minimize& initial_trust_region_radius(double value) {
        initial_trust_region_radius_ = value;
        return *this;
    }

    Minimize& final_trust_region_radius(double value) {
        final_trust_region_radius_ = value;
        return *this;
    }

    Minimize& max_function_calls_count(long value) {
        max_function_calls_count_ = value;
        return *this;
    }

private:
    static constexpr std::size_t variables_count_ = 2;
    static constexpr std::size_t number_of_interpolation_conditions_ = variables_count_ + 2;

    double initial_trust_region_radius_ = 1;
    double final_trust_region_radius_ = 1e3;
    long max_function_calls_count_ = std::numeric_limits<long>::max();
};

}

#endif
