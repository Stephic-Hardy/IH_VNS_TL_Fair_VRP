#include "vns_tabu.h"
#include "move.h"
#include "problem_arguments.hpp"
#include "neighborhood.h"

#include <chrono>
#include <algorithm>
#include <deque>
#include <quill/Logger.h>
#include <quill/LogMacros.h>

namespace {
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
}  // namespace

RoutePack VNSTabu::VnsTabuGlobal(const InputData& input_data, double ST [[maybe_unused]], int AON,
                                 int max_iterations_without_improve, double time_limit,
                                 const RoutePack& initial_solution, double alpha,
                                 quill::Logger* logger) {
    auto start_time = std::chrono::steady_clock::now();
    LOG_DEBUG(logger, "=================================================================");
    LOG_DEBUG(logger, "STARTING INTERROUTE FAIRNESS VNS+TABU ALGORITHM");
    LOG_DEBUG(logger, "=================================================================");

    RoutePack best_global = initial_solution;
    double best_global_max_cost = best_global.ComputeMaxDistance(input_data);

    RoutePack current = best_global;
    std::deque<TabuHash> tabu_list_moves;
    int iter_no_improve = 0;

    auto neighborhoods = GlobalNeighborhoods();

    auto penalty = [&](const RoutePack& solution) {
        double total_cost = solution.ComputeCost(input_data);
        double stdev = solution.ComputeDistanceStandardDeviation(input_data);
        return total_cost + alpha * stdev * solution.Size();
    };

    while (true) {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                           std::chrono::steady_clock::now() - start_time)
                           .count();
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

            TabuHash hash = move->GetTabuHash();
            bool in_tabu = (std::ranges::find(tabu_list_moves, hash) != tabu_list_moves.end());

            if (penalty(neighbor) < penalty(best_global) - 1e-9) {
                LOG_DEBUG(logger, "  *** GLOBAL FAIRNESS IMPROVED! MaxCost: {} -> {} alpha: {}",
                          best_global_max_cost, max_cost, alpha);
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

        while (tabu_list_moves.size() > static_cast<size_t>(AON)) {
            tabu_list_moves.pop_front();
        }
    }

    return best_global;
}
