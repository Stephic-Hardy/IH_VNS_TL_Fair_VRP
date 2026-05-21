#pragma once

#include "route.h"
#include "problem_arguments.hpp"

class InsertionHeuristic {
public:
    static Route BuildInitialTour(const std::vector<int> &global_subset, const InputData& input_data);
};
