#include <iostream>
#include "data_adapter.h"

bool validate_solution(const Tour& tour, const InputData& input_data, const std::vector<int>& new1_to_old0) {
    int n = input_data.points_count;
    
    if (!tour.validate(n)) {
        std::cerr << "Tour validation failed!" << std::endl;
        return false;
    }
    
    double total_time = tour.compute_cost(input_data, new1_to_old0);
    
    if (total_time > static_cast<double>(input_data.max_time)) {
        std::cout << "Warning: Solution exceeds max_time (" << total_time << " > " << input_data.max_time << ")" << std::endl;
        return false;
    }
    
    return true;
}