#include "../include/vns_tabu.h"
#include "../include/insertion_heuristic.h"
#include "problem_arguments.hpp"
#include "neighborhoods.h"
#include <random>
#include <chrono>
#include <algorithm>
#include <deque>
#include <iostream>
#include <iomanip>
#include <sstream>

std::deque<std::string> VNSTabu::tabu_list_moves;
std::deque<std::string> VNSTabu::tabu_list_2opt;

std::string VNSTabu::hash_move(int type, int i, int j, int k) {
    std::stringstream ss;
    ss << "MOVE_" << type << "_" << i << "_" << j << "_" << k;
    return ss.str();
}

std::string VNSTabu::hash_2opt(int i, int j) {
    std::stringstream ss;
    ss << "2OPT_" << i << "_" << j;
    return ss.str();
}

Tour VNSTabu::vns_without_tabu(const Tour& start_tour, const InputData& input_data, 
                              const std::vector<int>& new1_to_old0, int max_iter) {
    Tour best = start_tour.copy();
    double best_cost = best.compute_cost(input_data, new1_to_old0);
    double best_value = best.compute_value(input_data, new1_to_old0);
    double best_distance = best.compute_distance(input_data, new1_to_old0);

    Tour current = best.copy();

    for (int iter = 0; iter < max_iter; ++iter) {
        bool improved = false;
        
       
        for (int k = 0; k < 6; ++k) {
            NeighborhoodType type = static_cast<NeighborhoodType>(k);
            auto [neighbor, neighbor_cost] = Neighborhoods::find_best_neighbor(current, type, input_data, new1_to_old0, 5);
            double neighbor_value = neighbor.compute_value(input_data, new1_to_old0);
            double neighbor_distance = neighbor.compute_distance(input_data, new1_to_old0);
            
            double current_cost = current.compute_cost(input_data, new1_to_old0);
            double current_value = current.compute_value(input_data, new1_to_old0);
            double current_distance = current.compute_distance(input_data, new1_to_old0);
            
            // Сравнение: Value -> время -> расстояние
            if (neighbor_value > best_value + 1e-9 || 
               (std::abs(neighbor_value - best_value) < 1e-9 && neighbor_cost < best_cost - 1e-9) ||
               (std::abs(neighbor_value - best_value) < 1e-9 && std::abs(neighbor_cost - best_cost) < 1e-9 && neighbor_distance < best_distance - 1e-9)) {
                best = neighbor.copy();
                best_cost = neighbor_cost;
                best_value = neighbor_value;
                best_distance = neighbor_distance;
                current = best.copy();
                improved = true;
                break;
            } else if (neighbor_value > current_value + 1e-9 || 
                      (std::abs(neighbor_value - current_value) < 1e-9 && neighbor_cost < current_cost - 1e-9) ||
                      (std::abs(neighbor_value - current_value) < 1e-9 && std::abs(neighbor_cost - current_cost) < 1e-9 && neighbor_distance < current_distance - 1e-9)) {
                current = neighbor.copy();
                improved = true;
                break;
            }
        }
        if (!improved) {
            break;
        }
    }

    return best;
}


std::pair<Tour, double> VNSTabu::vns_tabu_advanced(int n, 
                                                  const InputData& input_data,
                                                  const std::vector<int>& new1_to_old0,
                                                  double ST,
                                                  int AON,
                                                  int max_iterations_without_improve,
                                                  int time_limit,
                                                  const Tour& initial_tour) {

    auto start_time = std::chrono::steady_clock::now();
    
    std::cout << "=================================================================" << std::endl;
    std::cout << "STARTING ADVANCED VNS+TABU ALGORITHM" << std::endl;
    std::cout << "Parameters: ST=" << ST << ", AON=" << AON 
              << ", MaxIterWithoutImprove=" << max_iterations_without_improve 
              << ", TimeLimit=" << time_limit << "s" << std::endl;
    std::cout << "=================================================================" << std::endl;

    Tour best_global = initial_tour.copy();
    double best_global_cost = best_global.compute_cost(input_data, new1_to_old0);
    double best_global_value = best_global.compute_value(input_data, new1_to_old0);
    double best_global_distance = best_global.compute_distance(input_data, new1_to_old0);
    Tour current = best_global.copy();

    tabu_list_moves.clear();
    tabu_list_2opt.clear();

    std::vector<Tour> LT;
    int iterations_without_global_improve = 0;
    int total_iterations = 0;
    bool global_improved_in_iteration = false;

    while (true) {
        total_iterations++;
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time).count();

        
        if (elapsed >= time_limit) {
            std::cout << "TIME LIMIT REACHED! Stopping." << std::endl;
            break;
        }
        
        if (iterations_without_global_improve >= max_iterations_without_improve) {
            std::cout << "MAX ITERATIONS WITHOUT GLOBAL IMPROVE REACHED! Stopping." << std::endl;
            break;
        }

        bool improved_in_neighborhood = false;
        global_improved_in_iteration = false;
        
        for (int k = 0; k < 6; ++k) {
            NeighborhoodType type = static_cast<NeighborhoodType>(k);
            
            auto [neighbor, neighbor_cost] = Neighborhoods::find_best_neighbor(current, type, input_data, new1_to_old0, 5);
            double neighbor_value = neighbor.compute_value(input_data, new1_to_old0);
            double neighbor_distance = neighbor.compute_distance(input_data, new1_to_old0);

            bool in_tabu = false;
            std::string move_hash;

            if (k < 3) {
                move_hash = hash_move(k, 0, 0, 0);
                in_tabu = (std::find(tabu_list_moves.begin(), tabu_list_moves.end(), move_hash) != tabu_list_moves.end());
            } else if (k == 3) {
                move_hash = hash_2opt(0, 0);
                in_tabu = (std::find(tabu_list_2opt.begin(), tabu_list_2opt.end(), move_hash) != tabu_list_2opt.end());
            }

            // Новый порядок оптимизации: Value -> время -> расстояние
            if (neighbor_value > best_global_value + 1e-9 || 
               (std::abs(neighbor_value - best_global_value) < 1e-9 && neighbor_cost < best_global_cost - 1e-9) ||
               (std::abs(neighbor_value - best_global_value) < 1e-9 && std::abs(neighbor_cost - best_global_cost) < 1e-9 && neighbor_distance < best_global_distance - 1e-9)) {

                if (neighbor_distance <= input_data.max_distance && neighbor_cost <= input_data.max_time) {
                    std::cout << "  *** GLOBAL IMPROVEMENT FOUND! ***" << std::endl;
                    std::cout << "  Old value: " << best_global_value << " -> New value: " << neighbor_value << std::endl;
                    std::cout << "  Old time: " << best_global_cost << " -> New time: " << neighbor_cost << std::endl;
                    std::cout << "  Distance: " << neighbor_distance << " (within limit " << input_data.max_distance << ")" << std::endl;
                    
                    best_global = neighbor.copy();
                    best_global_cost = neighbor_cost;
                    best_global_value = neighbor_value;
                    best_global_distance = neighbor_distance;
                    current = best_global.copy();
                    improved_in_neighborhood = true;
                    global_improved_in_iteration = true;

                    if (k < 3) {
                        tabu_list_moves.push_back(move_hash);
                    } else if (k == 3) {
                        tabu_list_2opt.push_back(move_hash);
                    }

                    iterations_without_global_improve = 0;
                    break;
                } else {
                    std::cout << "  *** VALUE/TIME IMPROVED BUT CONSTRAINTS VIOLATED! ***" << std::endl;
                    std::cout << "  Improved value: " << neighbor_value << " -> " << best_global_value << std::endl;
                    std::cout << "  Distance: " << neighbor_distance << " > limit " << input_data.max_distance 
                              << " or Time: " << neighbor_cost << " > limit " << input_data.max_time << std::endl;
                    continue;
                }
            } else if (!in_tabu) {
                double current_value = current.compute_value(input_data, new1_to_old0);
                double current_cost = current.compute_cost(input_data, new1_to_old0);
                double current_distance = current.compute_distance(input_data, new1_to_old0);
                
                // Новый порядок оптимизации: Value -> время -> расстояние
                if ((neighbor_value > current_value + 1e-9 || 
                    (std::abs(neighbor_value - current_value) < 1e-9 && neighbor_cost < current_cost - 1e-9) ||
                    (std::abs(neighbor_value - current_value) < 1e-9 && std::abs(neighbor_cost - current_cost) < 1e-9 && neighbor_distance < current_distance - 1e-9)) &&
                    neighbor_distance <= input_data.max_distance && neighbor_cost <= input_data.max_time) {
                    
                    current = neighbor.copy();
                    improved_in_neighborhood = true;

                    if (k < 3) {
                        tabu_list_moves.push_back(move_hash);
                    } else if (k == 3) {
                        tabu_list_2opt.push_back(move_hash);
                    }
                    break;
                } else if (neighbor_value > current_value + 1e-9 || 
                          (std::abs(neighbor_value - current_value) < 1e-9 && neighbor_cost < current_cost - 1e-9) ||
                          (std::abs(neighbor_value - current_value) < 1e-9 && std::abs(neighbor_cost - current_cost) < 1e-9 && neighbor_distance < current_distance - 1e-9)) {
                    std::cout << "  *** LOCAL IMPROVEMENT REJECTED - CONSTRAINTS VIOLATED ***" << std::endl;
                    std::cout << "  Distance: " << neighbor_distance << " > limit " << input_data.max_distance 
                              << " or Time: " << neighbor_cost << " > limit " << input_data.max_time << std::endl;
                    continue;
                }
            }
        }

        double current_cost = current.compute_cost(input_data, new1_to_old0);
        double current_value = current.compute_value(input_data, new1_to_old0);
        double threshold_value = best_global_value * (1.0 - ST); // Для value улучшение это увеличение, поэтому threshold ниже
        
        if (!improved_in_neighborhood) {
            if (current_value >= threshold_value) {
                LT.push_back(current.copy());
            }
            
            current = InsertionHeuristic::build_initial_tour(n, input_data, new1_to_old0);
        }

        if (LT.size() >= static_cast<size_t>(AON)) {
            std::vector<Tour> LT_VNS;
            for (size_t i = 0; i < LT.size(); ++i) {
                Tour improved = vns_without_tabu(LT[i], input_data, new1_to_old0, 50);
                LT_VNS.push_back(improved);
                
                double improved_cost = improved.compute_cost(input_data, new1_to_old0);
                double improved_value = improved.compute_value(input_data, new1_to_old0);
                double improved_distance = improved.compute_distance(input_data, new1_to_old0);

                // Новый порядок оптимизации: Value -> время -> расстояние
                if ((improved_value > best_global_value + 1e-9 || 
                    (std::abs(improved_value - best_global_value) < 1e-9 && improved_cost < best_global_cost - 1e-9) ||
                    (std::abs(improved_value - best_global_value) < 1e-9 && std::abs(improved_cost - best_global_cost) < 1e-9 && improved_distance < best_global_distance - 1e-9)) &&
                    improved_distance <= input_data.max_distance && improved_cost <= input_data.max_time) {
                    
                    best_global = improved.copy();
                    best_global_cost = improved_cost;
                    best_global_value = improved_value;
                    best_global_distance = improved_distance;
                    global_improved_in_iteration = true;
                    std::cout << "  *** VNS IMPROVED GLOBAL BEST! New value: " << best_global_value << " ***" << std::endl;
                } else if (improved_value > best_global_value + 1e-9 || 
                          (std::abs(improved_value - best_global_value) < 1e-9 && improved_cost < best_global_cost - 1e-9) ||
                          (std::abs(improved_value - best_global_value) < 1e-9 && std::abs(improved_cost - best_global_cost) < 1e-9 && improved_distance < best_global_distance - 1e-9)) {
                    std::cout << "  *** VNS IMPROVEMENT REJECTED - CONSTRAINTS VIOLATED ***" << std::endl;
                    std::cout << "  Improved value: " << improved_value << " Distance: " << improved_distance 
                            << " > limit " << input_data.max_distance 
                            << " or Time: " << improved_cost << " > limit " << input_data.max_time << std::endl;
                }
            }

            tabu_list_moves.clear();
            tabu_list_2opt.clear();
            
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<size_t> dist(0, LT_VNS.size() - 1);
            LT.clear();
            current = InsertionHeuristic::build_initial_tour(n, input_data, new1_to_old0);
            double new_cost = current.compute_cost(input_data, new1_to_old0);
            double new_value = current.compute_value(input_data, new1_to_old0);
            double new_distance = current.compute_distance(input_data, new1_to_old0);

            if ((new_value > best_global_value + 1e-9 || 
                (std::abs(new_value - best_global_value) < 1e-9 && new_cost < best_global_cost - 1e-9) ||
                (std::abs(new_value - best_global_value) < 1e-9 && std::abs(new_cost - best_global_cost) < 1e-9 && new_distance < best_global_distance - 1e-9)) &&
                new_distance <= input_data.max_distance && new_cost <= input_data.max_time) {
                
                best_global = current.copy();
                best_global_cost = new_cost;
                best_global_value = new_value;
                best_global_distance = new_distance;
                global_improved_in_iteration = true;
            } else if (new_value > best_global_value + 1e-9 || 
                      (std::abs(new_value - best_global_value) < 1e-9 && new_cost < best_global_cost - 1e-9) ||
                      (std::abs(new_value - best_global_value) < 1e-9 && std::abs(new_cost - best_global_cost) < 1e-9 && new_distance < best_global_distance - 1e-9)) {
                std::cout << "  *** INITIAL TOUR IMPROVEMENT REJECTED - CONSTRAINTS VIOLATED ***" << std::endl;
                std::cout << "  Improved value: " << new_value << " Distance: " << new_distance 
                        << " > limit " << input_data.max_distance 
                        << " or Time: " << new_cost << " > limit " << input_data.max_time << std::endl;
            }

            if (global_improved_in_iteration) {
                iterations_without_global_improve = 0;
            } else {
                iterations_without_global_improve++;
            }
        } else {
            if (global_improved_in_iteration) {
                iterations_without_global_improve = 0;
            }
        }

        if (tabu_list_moves.size() > static_cast<size_t>(AON)) {
            while (tabu_list_moves.size() > static_cast<size_t>(AON)) {
                tabu_list_moves.pop_front();
            }
        }

        if (tabu_list_2opt.size() > static_cast<size_t>(AON)) {
            while (tabu_list_2opt.size() > static_cast<size_t>(AON)) {
                tabu_list_2opt.pop_front();
            }
        }
    }

    auto end_time = std::chrono::steady_clock::now();
    auto total_elapsed = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();

    std::cout << "\n=================================================================" << std::endl;
    std::cout << "ALGORITHM FINISHED" << std::endl;
    std::cout << "Total iterations: " << total_iterations << std::endl;
    std::cout << "Total time: " << total_elapsed << "s" << std::endl;
    std::cout << "Final best value: " << best_global_value << std::endl;
    std::cout << "Final best cost: " << best_global_cost << std::endl;
    std::cout << "Total distance: " << best_global_distance << std::endl;
    std::cout << "=================================================================" << std::endl;

    return {best_global, best_global_cost};
}