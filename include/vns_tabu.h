#pragma once

#include "problem_arguments.hpp"
#include "route_pack.h"

#include <quill/Logger.h>

class VNSTabu {
public:
    static RoutePack VnsTabuAdvanced(const InputData& input_data, double ST, int AON,
                                     int max_iterations_without_improve, int time_limit,
                                     const RoutePack& initial_solution, size_t route,
                                     quill::Logger* logger);

    static RoutePack VnsWithoutTabu(const RoutePack& start_solution, const InputData& input_data,
                                    int max_iter, size_t route);

    static RoutePack VnsTabuGlobal(const InputData& input_data, double ST, int AON,
                                   int max_iterations_without_improve, int time_limit,
                                   const RoutePack& initial_solution, double alpha,
                                   quill::Logger* logger);
};
