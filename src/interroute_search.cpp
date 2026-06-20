#include "vns_tabu.h"
#include "move.h"
#include "problem_arguments.hpp"
#include "neighborhood.h"

#include <chrono>
#include <algorithm>
#include <cmath>
#include <deque>
#include <vector>
#include <quill/Logger.h>
#include <quill/LogMacros.h>

namespace {
std::vector<std::unique_ptr<Neighborhood>> GlobalNeighborhoods() {
    std::vector<std::unique_ptr<Neighborhood>> nh;
    nh.push_back(std::make_unique<InterRelocateNeighborhood>());
    nh.push_back(std::make_unique<InterSwapNeighborhood>());
    nh.push_back(std::make_unique<CrossExchangeNeighborhood>(4));
    nh.push_back(std::make_unique<TwoOptStarNeighborhood>());

    nh.push_back(std::make_unique<MoveVertexNeighborhood>());
    nh.push_back(std::make_unique<SwapNeighborhood>());
    return nh;
}

thread_local std::vector<SolutionMetrics> metrics_buffer;
const std::vector<SolutionMetrics> &GetMetrics(const RoutePack& pack, const InputData& input_data) {
    metrics_buffer.clear();
    metrics_buffer.reserve(pack.Size());
    for (size_t i = 0; i < pack.Size(); ++i) {
        metrics_buffer.push_back(pack.GetRoute(i).ComputeMetrics(input_data));
    }
    return metrics_buffer;
}
}  // namespace

RoutePack VNSTabu::VnsTabuGlobal(const InputData& input_data, double ST [[maybe_unused]], int AON,
                                 int max_iterations_without_improve, double time_limit,
                                 const RoutePack& initial_solution, double alpha,
                                 quill::Logger* logger, ExecutionStats& stats) {
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

    auto penalty = [&](const std::vector<SolutionMetrics>& metrics) {
        double total_cost = 0;
        double sum_dist = 0;
        double sum_sq_dist = 0;

        for (const auto& m : metrics) {
            total_cost += m.cost;
            sum_dist += m.distance;
            sum_sq_dist += m.distance * m.distance;
        }

        double n = static_cast<double>(metrics.size());
        double mean = sum_dist / n;
        double variance = std::max(0.0, (sum_sq_dist / n) - (mean * mean));
        double stdev = std::sqrt(variance);

        return total_cost + alpha * stdev * n;
    };
    double best_global_penalty = penalty(GetMetrics(best_global, input_data));

    while (true) {
        ++stats.global_vns_iterations;
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

            double neighbor_penalty = penalty(GetMetrics(neighbor, input_data));
            if (neighbor_penalty < best_global_penalty - 1e-9) {
                LOG_DEBUG(logger, "  *** GLOBAL FAIRNESS IMPROVED! MaxCost: {} -> {} alpha: {}",
                          best_global_max_cost, max_cost, alpha);
                best_global = neighbor;
                best_global_max_cost = max_cost;
                best_global_penalty = neighbor_penalty;
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
    
    auto end_time = std::chrono::steady_clock::now();
    stats.global_vns_time += std::chrono::duration<double>(end_time - start_time).count();

    return best_global;
}
