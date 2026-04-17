#include "../include/neighborhoods.h"
#include <limits>     
#include <algorithm>  
#include <iostream> 

std::pair<Tour, double> Neighborhoods::find_best_neighbor(const Tour& current, NeighborhoodType type,
                                                         const InputData& input_data, int k) {
    Tour best = current.copy();
    if (!best.validate()) {
        std::cerr << "Invalid initial tour in find_best_neighbor" << std::endl;
        return {best, current.compute_cost(input_data)};
    }
    
    double min_cost = current.compute_cost(input_data);
    double best_value = current.compute_value(input_data);
    double best_distance = current.compute_distance(input_data);
    size_t n = current.vertices.size();

    switch (type) {
        case N1_REMOVE_INSERT:
            for (size_t i = 1; i < n - 1; ++i) {
                Tour neighbor = current.copy();
                int v = neighbor.vertices[i];
                neighbor.vertices.erase(neighbor.vertices.begin() + i);
                neighbor.vertices.push_back(v);
                if (!neighbor.validate()) {
                    continue;
                }
                neighbor.invalidate_cache();
                double cost = neighbor.compute_cost(input_data);
                double value = neighbor.compute_value(input_data);
                double distance = neighbor.compute_distance(input_data);
                

                if (value > best_value + 1e-9 || 
                   (std::abs(value - best_value) < 1e-9 && cost < min_cost - 1e-9) ||
                   (std::abs(value - best_value) < 1e-9 && std::abs(cost - min_cost) < 1e-9 && distance < best_distance - 1e-9)) {
                    min_cost = cost;
                    best_value = value;
                    best_distance = distance;
                    best = neighbor;
                }
            }
            break;

        case N2_SWAP_ADJ:  
            for (size_t i = 1; i < n - 1; ++i) {
                Tour neighbor = current.copy();
                std::swap(neighbor.vertices[i], neighbor.vertices[i + 1]);
                if (!neighbor.validate()) {
                    continue;
                }
                neighbor.invalidate_cache();
                double cost = neighbor.compute_cost(input_data);
                double value = neighbor.compute_value(input_data);
                double distance = neighbor.compute_distance(input_data);
                
                if (value > best_value + 1e-9 || 
                   (std::abs(value - best_value) < 1e-9 && cost < min_cost - 1e-9) ||
                   (std::abs(value - best_value) < 1e-9 && std::abs(cost - min_cost) < 1e-9 && distance < best_distance - 1e-9)) {
                    min_cost = cost;
                    best_value = value;
                    best_distance = distance;
                    best = neighbor;

                }
            }
            break;

        case N3_SWAP:  
            for (size_t i = 1; i < n; ++i) {
                for (size_t j = i + 1; j < n; ++j) {
                    Tour neighbor = current.copy();
                    std::swap(neighbor.vertices[i], neighbor.vertices[j]);
                    if (!neighbor.validate()) {
                        continue;
                    }
                    neighbor.invalidate_cache();
                    double cost = neighbor.compute_cost(input_data);
                    double value = neighbor.compute_value(input_data);
                    double distance = neighbor.compute_distance(input_data);
                    
                    if (value > best_value + 1e-9 || 
                       (std::abs(value - best_value) < 1e-9 && cost < min_cost - 1e-9) ||
                       (std::abs(value - best_value) < 1e-9 && std::abs(cost - min_cost) < 1e-9 && distance < best_distance - 1e-9)) {
                        min_cost = cost;
                        best_value = value;
                        best_distance = distance;
                        best = neighbor;
                    }
                }
            }
            break;

        case N4_2OPT:  
            for (size_t i = 1; i < n - 2; ++i) {
                for (size_t j = i + 2; j < n; ++j) {
                    Tour neighbor = current.copy();
                    std::reverse(neighbor.vertices.begin() + i, neighbor.vertices.begin() + j + 1);
                    if (!neighbor.validate()) {
                        continue;
                    }
                    neighbor.invalidate_cache();
                    double cost = neighbor.compute_cost(input_data);
                    double value = neighbor.compute_value(input_data);
                    double distance = neighbor.compute_distance(input_data);
                    
                    if (value > best_value + 1e-9 || 
                       (std::abs(value - best_value) < 1e-9 && cost < min_cost - 1e-9) ||
                       (std::abs(value - best_value) < 1e-9 && std::abs(cost - min_cost) < 1e-9 && distance < best_distance - 1e-9)) {
                        min_cost = cost;
                        best_value = value;
                        best_distance = distance;
                        best = neighbor;
                    }
                }
            }
            break;

        case N5_MOVE_FWD_K:  
            for (size_t i = 2; i <= n - k; ++i) {  
                if (i - 1 < k) continue; 
                if (i + k > n) continue;
                if (i >= current.vertices.size()) continue;
                if (i + k > current.vertices.size()) continue;
                
                Tour neighbor = current.copy();
                auto block = std::vector<int>(neighbor.vertices.begin() + i, neighbor.vertices.begin() + i + k);
                
                if (i < neighbor.vertices.size() && i + k <= neighbor.vertices.size()) {
                    neighbor.vertices.erase(neighbor.vertices.begin() + i, neighbor.vertices.begin() + i + k);
                } else {
                    continue;
                }
                
                size_t insert_pos = i + 1;
                if (insert_pos <= neighbor.vertices.size()) {
                    neighbor.vertices.insert(neighbor.vertices.begin() + insert_pos, block.begin(), block.end());
                } else {
                    continue;
                }
                
                if (!neighbor.validate()) {
                    continue;
                }
                
                neighbor.invalidate_cache();
                double cost = neighbor.compute_cost(input_data);
                double value = neighbor.compute_value(input_data);
                double distance = neighbor.compute_distance(input_data);
                
                if (value > best_value + 1e-9 || 
                   (std::abs(value - best_value) < 1e-9 && cost < min_cost - 1e-9) ||
                   (std::abs(value - best_value) < 1e-9 && std::abs(cost - min_cost) < 1e-9 && distance < best_distance - 1e-9)) {
                    min_cost = cost;
                    best_value = value;
                    best_distance = distance;
                    best = neighbor;
                }
            }
            break;

        case N6_MOVE_BWD_K: 
            for (size_t i = k + 1; i < n; ++i) {  
                if (i - k < 1) continue;  
                Tour neighbor = current.copy();
                auto block = std::vector<int>(neighbor.vertices.begin() + i - k + 1, neighbor.vertices.begin() + i + 1);
                neighbor.vertices.erase(neighbor.vertices.begin() + i - k + 1, neighbor.vertices.begin() + i + 1);
                neighbor.vertices.insert(neighbor.vertices.begin() + i - k, block.begin(), block.end());
                if (!neighbor.validate()) {
                    continue;
                }
                neighbor.invalidate_cache();
                double cost = neighbor.compute_cost(input_data);
                double value = neighbor.compute_value(input_data);
                double distance = neighbor.compute_distance(input_data);
                
                if (value > best_value + 1e-9 || 
                   (std::abs(value - best_value) < 1e-9 && cost < min_cost - 1e-9) ||
                   (std::abs(value - best_value) < 1e-9 && std::abs(cost - min_cost) < 1e-9 && distance < best_distance - 1e-9)) {
                    min_cost = cost;
                    best_value = value;
                    best_distance = distance;
                    best = neighbor;
                }
            }
            break;

        default:
            break;
    }

    if (!best.validate()) {
        std::cerr << "Final best neighbor invalid!" << std::endl;
    }
    return {best, min_cost};
}