#include "vns_tabu.h"
#include "move.h"
#include "problem_arguments.hpp"
#include "neighborhood.h"

#include <random>
#include <chrono>
#include <algorithm>
#include <deque>
#include <iostream>

namespace {
std::deque<std::string> tabu_list_moves;

std::vector<std::unique_ptr<Neighborhood>> GlobalNeighborhoods() {
    std::vector<std::unique_ptr<Neighborhood>> nh;
    nh.push_back(std::make_unique<InterRelocateNeighborhood>());
    nh.push_back(std::make_unique<InterSwapNeighborhood>());
    nh.push_back(std::make_unique<CrossExchangeNeighborhood>());
    nh.push_back(std::make_unique<TwoOptStarNeighborhood>());

    nh.push_back(std::make_unique<MoveVertexNeighborhood>());
    nh.push_back(std::make_unique<SwapNeighborhood>());
    return nh;
}
}


RoutePack VNSTabu::VnsTabuGlobal(const InputData& input_data, double ST[[maybe_unused]], int AON,
                                 int max_iterations_without_improve, int time_limit,
                                 const RoutePack& initial_solution) {
    auto start_time = std::chrono::steady_clock::now();
    std::cout << "=================================================================" << std::endl;
    std::cout << "STARTING INTERROUTE FAIRNESS VNS+TABU ALGORITHM" << std::endl;
    std::cout << "=================================================================" << std::endl;

    RoutePack best_global = initial_solution;
    double best_global_max_cost = best_global.ComputeMaxDistance(input_data);

    RoutePack current = best_global;
    tabu_list_moves.clear();
    int iter_no_improve = 0;

    auto neighborhoods = GlobalNeighborhoods();

    // TODO: move to the command line arguments
    double alpha = 0.4;
    auto penalty = [&](const RoutePack& solution) {
        double total_cost = solution.ComputeCost(input_data);
        double stdev = solution.ComputeDistanceStandardDeviation(input_data);
        return total_cost + alpha * stdev * solution.Size();
    };

    while (true) {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - start_time).count();
        if (elapsed >= time_limit || iter_no_improve >= max_iterations_without_improve) {
            break;
        }

        bool improved_in_nh = false;

        for (auto& nb : neighborhoods) {
            auto [neighbor, move] = nb->FindBestNeighbor(current, input_data, penalty);
            if (!move) {
                continue;
            }

            double max_cost = neighbor.ComputeMaxDistance(input_data);

            std::string hash = move->GetTabuHash();
            bool in_tabu = (std::find(tabu_list_moves.begin(), tabu_list_moves.end(), hash) !=
                            tabu_list_moves.end());

            if (penalty(neighbor) < penalty(best_global) - 1e-9) {
                std::cout << "  *** GLOBAL FAIRNESS IMPROVED! MaxCost: " << best_global_max_cost <<
                    " -> " << max_cost << " alpha: " << alpha << std::endl;
                best_global = neighbor;
                best_global_max_cost = max_cost;
                current = neighbor;
                improved_in_nh = true;
                tabu_list_moves.push_back(hash);
                iter_no_improve = 0;
                break;
            } else if (!in_tabu) {
                current = neighbor;
                improved_in_nh = true;
                tabu_list_moves.push_back(hash);
                break;
            }
        }

        if (!improved_in_nh) {
            current = best_global;
            iter_no_improve++;
        }

        while (tabu_list_moves.size() > static_cast<size_t>(AON * 2)) {
            tabu_list_moves.pop_front();
        }
    }

    return best_global;
}