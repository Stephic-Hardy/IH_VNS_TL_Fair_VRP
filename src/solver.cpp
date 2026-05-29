#include "solver.h"
#include "problem_arguments.hpp"
#include "route_pack.h"
#include "initializer.h"
#include "optimizer.h"

#include <iostream>
#include <chrono>
#include <memory>

Solver::Solver(std::unique_ptr<Initializer> initializer, std::unique_ptr<Optimizer> optimizer)
    : initializer_(std::move(initializer)), optimizer_(std::move(optimizer)) {
}

Solution Solver::Solve(const InputData& input_data) {
    auto start_timer = std::chrono::high_resolution_clock::now();

    std::cout << "Building initial routes..." << std::endl;
    RoutePack routes = initializer_->BuildInitialRoutes(input_data);

    std::cout << "Optimizing routes..." << std::endl;
    routes = optimizer_->Optimize(routes, input_data);

    auto end_timer = std::chrono::high_resolution_clock::now();
    double exec_time = std::chrono::duration<double>(end_timer - start_timer).count();

    std::vector<AgentSolution> all_agent_solutions;
    for (const auto& tour : routes.Routes()) {
        AgentSolution sol;
        sol.route = std::vector<uint64_t>(tour->Vertices().begin(), tour->Vertices().end());
        if (sol.route.back() != 0) {
            sol.route.push_back(0);
        }
        sol.solution_size = sol.route.size();
        sol.total_time = tour->ComputeCost(input_data);
        sol.total_distance = tour->ComputeDistance(input_data);
        sol.total_value = tour->ComputeValue(input_data);
        all_agent_solutions.push_back(sol);
    }

    BenchmarkMetadata meta{};
    meta.execution_time = exec_time;

    return Solution{all_agent_solutions, meta};
}

BaselineSolver::BaselineSolver(double st, int aon, int max_iter, int time_limit)
    : Solver(std::make_unique<InsertionHeuristicInitializer>(),
             std::make_unique<BaselineOptimizer>(st, aon, max_iter, time_limit)) {
}

AnnealingSolver::AnnealingSolver(double st, int aon, int max_iter, int time_limit, double alpha)
    : Solver(std::make_unique<InsertionHeuristicInitializer>(),
             std::make_unique<AnnealingOptimizer>(st, aon, max_iter, time_limit, alpha)) {
}

RebalancingSolver::RebalancingSolver(double st, int aon, int max_iter, int time_limit,
                                     double fairness)
    : Solver(std::make_unique<InsertionHeuristicInitializer>(),
             std::make_unique<RebalancingOptimizer>(st, aon, max_iter, time_limit, fairness)) {
}
