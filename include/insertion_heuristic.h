#pragma once

#include "tour.h"
#include "problem_arguments.hpp"

class InsertionHeuristic {
public:
    static Tour BuildInitialTour(const std::vector<int> &global_subset, const InputData& input_data);
};
