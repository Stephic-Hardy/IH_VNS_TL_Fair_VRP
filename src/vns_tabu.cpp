#include "vns_tabu.h"
#include "move.h"
#include "problem_arguments.hpp"
#include "insertion_heuristic.h"
#include "neighborhood.h"

#include <random>
#include <chrono>
#include <algorithm>
#include <deque>

#include <quill/LogMacros.h>

namespace {
std::vector<std::unique_ptr<Neighborhood>> Neighborhoods() {
    std::vector<std::unique_ptr<Neighborhood>> neighborhoods;
    neighborhoods.push_back(std::make_unique<RemovePushBackNeighborhood>());
    neighborhoods.push_back(std::make_unique<SwapAdjNeighborhood>());
    neighborhoods.push_back(std::make_unique<SwapNeighborhood>());
    neighborhoods.push_back(std::make_unique<TwoOptNeighborhood>());
    neighborhoods.push_back(std::make_unique<BlockMoveForwardNeighborhood>(5));
    neighborhoods.push_back(std::make_unique<BlockMoveBackwardNeighborhood>(5));
    return neighborhoods;
}
}  // namespace

RoutePack VNSTabu::VnsWithoutTabu(const RoutePack& start_solution, const InputData& input_data,
                                  int max_iter, size_t route) {
    RoutePack best = start_solution;
    double best_cost = best.GetRoute(route).ComputeCost(input_data);
    double best_value = best.GetRoute(route).ComputeValue(input_data);
    double best_distance = best.GetRoute(route).ComputeDistance(input_data);

    RoutePack current = best;

    auto neighborhoods = Neighborhoods();

    for (int iter = 0; iter < max_iter; ++iter) {
        bool improved = false;

        for (auto& neighborhood : neighborhoods) {
            auto [neighbor, _] = neighborhood->FindBestNeighbor(current, input_data, route);
            double neighbor_cost = neighbor.GetRoute(route).ComputeCost(input_data);
            double neighbor_value = neighbor.GetRoute(route).ComputeValue(input_data);
            double neighbor_distance = neighbor.GetRoute(route).ComputeDistance(input_data);

            double current_cost = current.GetRoute(route).ComputeCost(input_data);
            double current_value = current.GetRoute(route).ComputeValue(input_data);
            double current_distance = current.GetRoute(route).ComputeDistance(input_data);

            // Сравнение: Value -> время -> расстояние
            if (neighbor_value > best_value + 1e-9 ||
                (std::abs(neighbor_value - best_value) < 1e-9 &&
                 neighbor_cost < best_cost - 1e-9) ||
                (std::abs(neighbor_value - best_value) < 1e-9 &&
                 std::abs(neighbor_cost - best_cost) < 1e-9 &&
                 neighbor_distance < best_distance - 1e-9)) {
                best = neighbor;
                best_cost = neighbor_cost;
                best_value = neighbor_value;
                best_distance = neighbor_distance;
                current = best;
                improved = true;
                break;
            } else if (neighbor_value > current_value + 1e-9 ||
                       (std::abs(neighbor_value - current_value) < 1e-9 &&
                        neighbor_cost < current_cost - 1e-9) ||
                       (std::abs(neighbor_value - current_value) < 1e-9 &&
                        std::abs(neighbor_cost - current_cost) < 1e-9 &&
                        neighbor_distance < current_distance - 1e-9)) {
                current = neighbor;
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

RoutePack VNSTabu::VnsTabuAdvanced(const InputData& input_data, double ST, int AON,
                                   int max_iterations_without_improve, int time_limit,
                                   const RoutePack& initial_solution, size_t route,
                                   quill::Logger* logger) {

    auto start_time = std::chrono::steady_clock::now();

    LOG_DEBUG(logger, "=================================================================");
    LOG_DEBUG(logger, "STARTING ADVANCED VNS+TABU ALGORITHM");
    LOG_DEBUG(logger, "Parameters: ST={}, AON={}, MaxIterWithoutImprove={}, TimeLimit={}s", ST, AON,
              max_iterations_without_improve, time_limit);
    LOG_DEBUG(logger, "=================================================================");

    std::vector<int> agent_subset;
    for (int v : initial_solution.GetRoute(route).Vertices()) {
        if (v != 0) {
            agent_subset.push_back(v);
        }
    }

    RoutePack best_global = initial_solution;
    double best_global_cost = best_global.GetRoute(route).ComputeCost(input_data);
    double best_global_value = best_global.GetRoute(route).ComputeValue(input_data);
    double best_global_distance = best_global.GetRoute(route).ComputeDistance(input_data);
    RoutePack current = best_global;

    
    std::deque<std::string> tabu_list_moves;
    std::deque<std::string> tabu_list_2opt;

    std::vector<RoutePack> LT;
    int iterations_without_global_improve = 0;
    int total_iterations = 0;
    bool global_improved_in_iteration = false;

    auto neighborhoods = Neighborhoods();

    while (true) {
        total_iterations++;
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed =
            std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time).count();

        if (elapsed >= time_limit) {
            LOG_DEBUG(logger, "TIME LIMIT REACHED! Stopping.");
            break;
        }

        if (iterations_without_global_improve >= max_iterations_without_improve) {
            LOG_DEBUG(logger, "MAX ITERATIONS WITHOUT GLOBAL IMPROVE REACHED! Stopping.");
            break;
        }

        bool improved_in_neighborhood = false;
        global_improved_in_iteration = false;

        for (auto& neighborhood : neighborhoods) {
            auto [neighbor, move] = neighborhood->FindBestNeighbor(current, input_data, route);

            if (!move) {
                continue;
            }

            double neighbor_cost = neighbor.GetRoute(route).ComputeCost(input_data);
            double neighbor_value = neighbor.GetRoute(route).ComputeValue(input_data);
            double neighbor_distance = neighbor.GetRoute(route).ComputeDistance(input_data);

            bool in_tabu = false;
            std::string move_hash;

            move_hash = move->GetTabuHash();
            in_tabu = (std::find(tabu_list_moves.begin(), tabu_list_moves.end(), move_hash) !=
                       tabu_list_moves.end());

            // Новый порядок оптимизации: Value -> время -> расстояние
            if (neighbor_value > best_global_value + 1e-9 ||
                (std::abs(neighbor_value - best_global_value) < 1e-9 &&
                 neighbor_cost < best_global_cost - 1e-9) ||
                (std::abs(neighbor_value - best_global_value) < 1e-9 &&
                 std::abs(neighbor_cost - best_global_cost) < 1e-9 &&
                 neighbor_distance < best_global_distance - 1e-9)) {

                if (neighbor_distance <= input_data.max_distance &&
                    neighbor_cost <= input_data.max_time) {
                    LOG_DEBUG(logger, "  *** GLOBAL IMPROVEMENT FOUND! ***");
                    LOG_DEBUG(logger, "  Old value: {} -> New value: {}", best_global_value,
                              neighbor_value);
                    LOG_DEBUG(logger, "  Old time: {} -> New time: {}", best_global_cost,
                              neighbor_cost);
                    LOG_DEBUG(logger, "  Distance: {} (within limit {})", neighbor_distance,
                              input_data.max_distance);

                    best_global = neighbor;
                    best_global_cost = neighbor_cost;
                    best_global_value = neighbor_value;
                    best_global_distance = neighbor_distance;
                    current = best_global;
                    improved_in_neighborhood = true;
                    global_improved_in_iteration = true;

                    tabu_list_moves.push_back(move_hash);

                    iterations_without_global_improve = 0;
                    break;
                } else {
                    LOG_DEBUG(logger, "  *** VALUE/TIME IMPROVED BUT CONSTRAINTS VIOLATED! ***");
                    LOG_DEBUG(logger, "  Improved value: {} -> {}", neighbor_value,
                              best_global_value);
                    LOG_DEBUG(logger, "  Distance: {} > limit {} or Time: {} > limit {}",
                              neighbor_distance, input_data.max_distance, neighbor_cost,
                              input_data.max_time);
                    continue;
                }
            } else if (!in_tabu) {
                double current_value = current.GetRoute(route).ComputeValue(input_data);
                double current_cost = current.GetRoute(route).ComputeCost(input_data);
                double current_distance = current.GetRoute(route).ComputeDistance(input_data);

                // Новый порядок оптимизации: Value -> время -> расстояние
                if ((neighbor_value > current_value + 1e-9 ||
                     (std::abs(neighbor_value - current_value) < 1e-9 &&
                      neighbor_cost < current_cost - 1e-9) ||
                     (std::abs(neighbor_value - current_value) < 1e-9 &&
                      std::abs(neighbor_cost - current_cost) < 1e-9 &&
                      neighbor_distance < current_distance - 1e-9)) &&
                    neighbor_distance <= input_data.max_distance &&
                    neighbor_cost <= input_data.max_time) {

                    current = neighbor;
                    improved_in_neighborhood = true;

                    auto move_type = move->Type();
                    if (move_type == N1_REMOVE_INSERT || move_type == N2_SWAP_ADJ ||
                        move_type == N3_SWAP) {
                        tabu_list_moves.push_back(move_hash);
                    } else if (move_type == N4_2OPT) {
                        tabu_list_2opt.push_back(move_hash);
                    }
                    break;
                } else if (neighbor_value > current_value + 1e-9 ||
                           (std::abs(neighbor_value - current_value) < 1e-9 &&
                            neighbor_cost < current_cost - 1e-9) ||
                           (std::abs(neighbor_value - current_value) < 1e-9 &&
                            std::abs(neighbor_cost - current_cost) < 1e-9 &&
                            neighbor_distance < current_distance - 1e-9)) {
                    LOG_DEBUG(logger,
                              "  *** LOCAL IMPROVEMENT REJECTED - CONSTRAINTS VIOLATED ***");
                    LOG_DEBUG(logger, "  Distance: {} > limit {} or Time: {} > limit {}",
                              neighbor_distance, input_data.max_distance, neighbor_cost,
                              input_data.max_time);
                    continue;
                }
            }
        }

        double current_value = current.GetRoute(route).ComputeValue(input_data);
        double threshold_value = best_global_value * (1.0 - ST);
        // Для value улучшение это увеличение, поэтому threshold ниже

        if (!improved_in_neighborhood) {
            if (current_value >= threshold_value) {
                LT.push_back(current);
            }
            current.MutateRoute(route) =
                InsertionHeuristic::BuildInitialTour(agent_subset, input_data);
        }

        if (LT.size() >= static_cast<size_t>(AON)) {
            std::vector<RoutePack> LT_VNS;
            for (size_t i = 0; i < LT.size(); ++i) {
                RoutePack improved = VnsWithoutTabu(LT[i], input_data, 50, route);
                LT_VNS.push_back(improved);

                double improved_cost = improved.GetRoute(route).ComputeCost(input_data);
                double improved_value = improved.GetRoute(route).ComputeValue(input_data);
                double improved_distance = improved.GetRoute(route).ComputeDistance(input_data);

                // Новый порядок оптимизации: Value -> время -> расстояние
                if ((improved_value > best_global_value + 1e-9 ||
                     (std::abs(improved_value - best_global_value) < 1e-9 &&
                      improved_cost < best_global_cost - 1e-9) ||
                     (std::abs(improved_value - best_global_value) < 1e-9 &&
                      std::abs(improved_cost - best_global_cost) < 1e-9 &&
                      improved_distance < best_global_distance - 1e-9)) &&
                    improved_distance <= input_data.max_distance &&
                    improved_cost <= input_data.max_time) {

                    best_global = improved;
                    best_global_cost = improved_cost;
                    best_global_value = improved_value;
                    best_global_distance = improved_distance;
                    global_improved_in_iteration = true;
                    LOG_DEBUG(logger, "  *** VNS IMPROVED GLOBAL BEST! New value: {} ***",
                              best_global_value);
                } else if (improved_value > best_global_value + 1e-9 ||
                           (std::abs(improved_value - best_global_value) < 1e-9 &&
                            improved_cost < best_global_cost - 1e-9) ||
                           (std::abs(improved_value - best_global_value) < 1e-9 &&
                            std::abs(improved_cost - best_global_cost) < 1e-9 &&
                            improved_distance < best_global_distance - 1e-9)) {
                    LOG_DEBUG(logger, "  *** VNS IMPROVEMENT REJECTED - CONSTRAINTS VIOLATED ***");
                    LOG_DEBUG(logger,
                              "  Improved value: {} Distance: {} > limit {} or Time: {} > limit {}",
                              improved_value, improved_distance, input_data.max_distance,
                              improved_cost, input_data.max_time);
                }
            }

            tabu_list_moves.clear();
            tabu_list_2opt.clear();

            if (!LT_VNS.empty()) {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<size_t> dist(0, LT_VNS.size() - 1);
                current = LT_VNS[dist(gen)];
            }

            LT.clear();
            current.MutateRoute(route) =
                InsertionHeuristic::BuildInitialTour(agent_subset, input_data);
            double new_cost = current.GetRoute(route).ComputeCost(input_data);
            double new_value = current.GetRoute(route).ComputeValue(input_data);
            double new_distance = current.GetRoute(route).ComputeDistance(input_data);

            if ((new_value > best_global_value + 1e-9 ||
                 (std::abs(new_value - best_global_value) < 1e-9 &&
                  new_cost < best_global_cost - 1e-9) ||
                 (std::abs(new_value - best_global_value) < 1e-9 &&
                  std::abs(new_cost - best_global_cost) < 1e-9 &&
                  new_distance < best_global_distance - 1e-9)) &&
                new_distance <= input_data.max_distance && new_cost <= input_data.max_time) {

                best_global = current;
                best_global_cost = new_cost;
                best_global_value = new_value;
                best_global_distance = new_distance;
                global_improved_in_iteration = true;
            } else if (new_value > best_global_value + 1e-9 ||
                       (std::abs(new_value - best_global_value) < 1e-9 &&
                        new_cost < best_global_cost - 1e-9) ||
                       (std::abs(new_value - best_global_value) < 1e-9 &&
                        std::abs(new_cost - best_global_cost) < 1e-9 &&
                        new_distance < best_global_distance - 1e-9)) {
                LOG_DEBUG(logger,
                          "  *** INITIAL TOUR IMPROVEMENT REJECTED - CONSTRAINTS VIOLATED ***");
                LOG_DEBUG(logger,
                          "  Improved value: {} Distance: {} > limit {} or Time: {} > limit {}",
                          new_value, new_distance, input_data.max_distance, new_cost,
                          input_data.max_time);
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

        while (tabu_list_moves.size() > static_cast<size_t>(AON)) {
            tabu_list_moves.pop_front();
        }

        while (tabu_list_2opt.size() > static_cast<size_t>(AON)) {
            tabu_list_2opt.pop_front();
        }
    }

    auto end_time = std::chrono::steady_clock::now();
    auto total_elapsed =
        std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();

    LOG_DEBUG(logger, "\n=================================================================");
    LOG_DEBUG(logger, "ALGORITHM FINISHED");
    LOG_DEBUG(logger, "Total iterations: {}", total_iterations);
    LOG_DEBUG(logger, "Total time: {}s", total_elapsed);
    LOG_DEBUG(logger, "Final best value: {}", best_global_value);
    LOG_DEBUG(logger, "Final best cost: {}", best_global_cost);
    LOG_DEBUG(logger, "Total distance: {}", best_global_distance);
    LOG_DEBUG(logger, "=================================================================");
    return best_global;
}
