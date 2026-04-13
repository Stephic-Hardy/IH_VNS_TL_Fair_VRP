#include "insertion_heuristic.h"
#include <random>    
#include <chrono>    
#include <unordered_set>
#include <algorithm> 

Tour InsertionHeuristic::build_initial_tour(int n, const InputData& input_data, const std::vector<int>& new1_to_old0) {
    auto seed = std::chrono::steady_clock::now().time_since_epoch().count();
    std::mt19937 gen(static_cast<unsigned>(seed));

    Tour tour(n);
    tour.vertices = {1};  
    std::unordered_set<int> unvisited;  
    for (int i = 2; i <= n; ++i) {
        unvisited.insert(i);
    }

    while (!unvisited.empty()) {
        int rd = gen() % 2; 

        if (rd == 0) {  
            double min_delta = std::numeric_limits<double>::infinity();
            int best_vertex = -1;
            size_t best_position = 0;

            for (int v : unvisited) {
                for (size_t j = 1; j <= tour.vertices.size(); ++j) {  
                    Tour temp = tour.copy();
                    temp.vertices.insert(temp.vertices.begin() + j, v);
                    temp.invalidate_cache();
                    double delta = temp.compute_cost(input_data, new1_to_old0) - tour.compute_cost(input_data, new1_to_old0);
                    if (delta < min_delta) {
                        min_delta = delta;
                        best_vertex = v;
                        best_position = j;
                    }
                }
            }
            tour.vertices.insert(tour.vertices.begin() + best_position, best_vertex);
            unvisited.erase(best_vertex);

        } else {  
            double max_min_dist = -1.0;
            int best_vertex = -1;

            for (int v : unvisited) {
                double min_dist = std::numeric_limits<double>::infinity();
                for (int u : tour.vertices) {
                    double dist = input_data.get_time_dependent_cost(0, new1_to_old0[u], new1_to_old0[v]);
                    min_dist = std::min(min_dist, dist);
                }
                if (min_dist > max_min_dist) {
                    max_min_dist = min_dist;
                    best_vertex = v;
                }
            }

            double min_delta = std::numeric_limits<double>::infinity();
            size_t best_position = 0;
            for (size_t j = 1; j <= tour.vertices.size(); ++j) {
                Tour temp = tour.copy();
                temp.vertices.insert(temp.vertices.begin() + j, best_vertex);
                temp.invalidate_cache();
                double delta = temp.compute_cost(input_data, new1_to_old0) - tour.compute_cost(input_data, new1_to_old0);
                if (delta < min_delta) {
                    min_delta = delta;
                    best_position = j;
                }
            }

            tour.vertices.insert(tour.vertices.begin() + best_position, best_vertex);
            unvisited.erase(best_vertex);
        }
    }

    tour.invalidate_cache(); 
    return tour;
}