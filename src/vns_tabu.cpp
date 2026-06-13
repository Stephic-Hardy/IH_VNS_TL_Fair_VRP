#include "vns_tabu.h"
#include "move.h"
#include "problem_arguments.hpp"
#include "insertion_heuristic.h"
#include "neighborhood.h"
#include "solution_metrics.h"

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

    SolutionMetrics best_metrics = {
        best.GetRoute(route).ComputeCost(input_data),
        best.GetRoute(route).ComputeValue(input_data),
        best.GetRoute(route).ComputeDistance(input_data),
    };
    RoutePack current = best;

    auto neighborhoods = Neighborhoods();

    for (int iter = 0; iter < max_iter; ++iter) {
        bool improved = false;

        for (auto& neighborhood : neighborhoods) {
            auto [neighbor, _] = neighborhood->FindBestNeighbor(current, input_data, route);

            SolutionMetrics neighbor_metrics = {
                neighbor.GetRoute(route).ComputeCost(input_data),
                neighbor.GetRoute(route).ComputeValue(input_data),
                neighbor.GetRoute(route).ComputeDistance(input_data),
            };

            SolutionMetrics current_metrics = {
                current.GetRoute(route).ComputeCost(input_data),
                current.GetRoute(route).ComputeValue(input_data),
                current.GetRoute(route).ComputeDistance(input_data),
            };

            if (neighbor_metrics > best_metrics) {
                best = neighbor;
                best_metrics = neighbor_metrics;
                current = best;
                improved = true;
                break;
            } else if (neighbor_metrics > current_metrics) {
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
                                   int max_iterations_without_improve, double time_limit,
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
    SolutionMetrics best_metrics = {
        best_global.GetRoute(route).ComputeValue(input_data),
        best_global.GetRoute(route).ComputeCost(input_data),
        best_global.GetRoute(route).ComputeDistance(input_data),
    };
    RoutePack current = best_global;

    std::deque<TabuHash> tabu_list_moves;
    std::deque<TabuHash> tabu_list_2opt;

    std::vector<RoutePack> LT;
    int iterations_without_global_improve = 0;
    int total_iterations = 0;
    bool global_improved_in_iteration = false;

    std::random_device rnd_device;
    std::mt19937 rnd_generator(rnd_device());

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

            SolutionMetrics neighbor_metrics = {
                neighbor.GetRoute(route).ComputeCost(input_data),
                neighbor.GetRoute(route).ComputeValue(input_data),
                neighbor.GetRoute(route).ComputeDistance(input_data),
            };

            bool in_tabu = false;

            TabuHash move_hash = move->GetTabuHash();
            MoveType move_type = move->Type();
            in_tabu = (std::ranges::find(tabu_list_moves, move_hash) != tabu_list_moves.end());

            if (neighbor_metrics > best_metrics) {
                LOG_DEBUG(logger, "  *** GLOBAL IMPROVEMENT FOUND! ***");
                LOG_DEBUG(logger, "  Old value: {} -> New value: {}", best_metrics.value,
                          neighbor_metrics.value);
                LOG_DEBUG(logger, "  Old time: {} -> New time: {}", best_metrics.cost,
                          neighbor_metrics.cost);
                LOG_DEBUG(logger, "  Distance: {} (within limit {})", neighbor_metrics.distance,
                          input_data.max_distance);

                best_global = neighbor;
                best_metrics = neighbor_metrics;

                current = best_global;
                improved_in_neighborhood = true;
                global_improved_in_iteration = true;

                if (move_type == N1_REMOVE_INSERT || move_type == N2_SWAP_ADJ ||
                    move_type == N3_SWAP) {
                    tabu_list_moves.push_back(move_hash);
                } else if (move_type == N4_2OPT) {
                    tabu_list_2opt.push_back(move_hash);
                }

                iterations_without_global_improve = 0;
                break;
            }

            SolutionMetrics current_metrics = {
                current.GetRoute(route).ComputeValue(input_data),
                current.GetRoute(route).ComputeCost(input_data),
                current.GetRoute(route).ComputeDistance(input_data),
            };

            if (!in_tabu && neighbor_metrics > current_metrics) {
                current = neighbor;
                improved_in_neighborhood = true;

                if (move_type == N1_REMOVE_INSERT || move_type == N2_SWAP_ADJ ||
                    move_type == N3_SWAP) {
                    tabu_list_moves.push_back(move_hash);
                } else if (move_type == N4_2OPT) {
                    tabu_list_2opt.push_back(move_hash);
                }
                break;
            }
        }

        double current_value = current.GetRoute(route).ComputeValue(input_data);
        double threshold_value = best_metrics.value * (1.0 - ST);

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

                SolutionMetrics improved_metrics = {
                    improved.GetRoute(route).ComputeCost(input_data),
                    improved.GetRoute(route).ComputeValue(input_data),
                    improved.GetRoute(route).ComputeDistance(input_data),
                };

                if (improved_metrics > best_metrics) {
                    best_global = improved;
                    best_metrics = improved_metrics;
                    global_improved_in_iteration = true;
                    LOG_DEBUG(logger, "  *** VNS IMPROVED GLOBAL BEST! New value: {} ***",
                              best_metrics.value);
                }
            }

            tabu_list_moves.clear();
            tabu_list_2opt.clear();

            if (!LT_VNS.empty()) {
                std::uniform_int_distribution<size_t> dist(0, LT_VNS.size() - 1);
                current = LT_VNS[dist(rnd_generator)];
            }

            LT.clear();
            current.MutateRoute(route) =
                InsertionHeuristic::BuildInitialTour(agent_subset, input_data);

            SolutionMetrics new_metrics = {
                current.GetRoute(route).ComputeCost(input_data),
                current.GetRoute(route).ComputeValue(input_data),
                current.GetRoute(route).ComputeDistance(input_data),
            };

            if (new_metrics > best_metrics) {
                best_global = current;
                best_metrics = new_metrics;
                global_improved_in_iteration = true;
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
    LOG_DEBUG(logger, "Final best value: {}", best_metrics.value);
    LOG_DEBUG(logger, "Final best cost: {}", best_metrics.cost);
    LOG_DEBUG(logger, "Total distance: {}", best_metrics.distance);
    LOG_DEBUG(logger, "=================================================================");
    return best_global;
}
