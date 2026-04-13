#ifndef INSERTION_HEURISTIC_H
#define INSERTION_HEURISTIC_H

#include "tour.h"
#include "problem_arguments.hpp"

class InsertionHeuristic {
public:
    static Tour build_initial_tour(int n, const InputData& input_data, const std::vector<int>& new1_to_old0);
};

#endif