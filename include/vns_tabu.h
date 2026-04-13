#ifndef VNS_TABU_H
#define VNS_TABU_H

#include "tour.h"
#include "problem_arguments.hpp"
#include <vector>
#include <deque>
#include <string>

class VNSTabu {
private:
    static std::deque<std::string> tabu_list_moves;
    static std::deque<std::string> tabu_list_2opt;
    
    static std::string hash_move(int type, int i, int j, int k);
    static std::string hash_2opt(int i, int j);
    static Tour vns_without_tabu(const Tour& start_tour, const InputData& input_data, 
                                const std::vector<int>& new1_to_old0, int max_iter);


public:
    static std::pair<Tour, double> vns_tabu_advanced(
        int n,
        const InputData& input_data,
        const std::vector<int>& new1_to_old0,
        double ST,
        int AON,
        int max_iterations_without_improve,
        int time_limit,
        const Tour& initial_tour
    );
};

#endif