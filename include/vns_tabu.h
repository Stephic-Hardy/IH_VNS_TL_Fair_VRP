#pragma once

#include "problem_arguments.hpp"
#include "route_pack.h"

class VNSTabu {
public:
    static RoutePack VnsTabuAdvanced(
        const InputData& input_data,
        double ST,
        int AON,
        int max_iterations_without_improve,
        int time_limit,
        const RoutePack& initial_solution,
        size_t route
        );

    static RoutePack VnsWithoutTabu(const RoutePack& start_solution, const InputData& input_data,
                                    int max_iter, size_t route);

    static RoutePack VnsTabuGlobal(
        const InputData& input_data, double ST, int AON,
        int max_iterations_without_improve, int time_limit, const RoutePack& initial_solution
        );
};