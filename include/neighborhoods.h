#ifndef NEIGHBORHOODS_H
#define NEIGHBORHOODS_H

#include "tour.h"
#include "problem_arguments.hpp"

enum NeighborhoodType {
    N1_REMOVE_INSERT,
    N2_SWAP_ADJ,
    N3_SWAP,
    N4_2OPT,
    N5_MOVE_FWD_K,
    N6_MOVE_BWD_K
};

class Neighborhoods {
public:
    static std::pair<Tour, double> find_best_neighbor(
        const Tour& current, 
        NeighborhoodType type,
        const InputData& input_data,
        const std::vector<int>& new1_to_old0,
        int k = 3
    );
};

#endif