#include "solver.h"
#include "problem_arguments.hpp"
#include "route_pack.h"
#include "initializer.h"
#include "utils.h"
#include "optimizer.h"

#include <quill/Backend.h>
#include <quill/Frontend.h>
#include <quill/LogMacros.h>
#include <quill/sinks/ConsoleSink.h>

#include <cassert>
#include <chrono>
#include <memory>

Solver::Solver(std::unique_ptr<Initializer> initializer, std::unique_ptr<Optimizer> optimizer,
               int verbose)
    : initializer_(std::move(initializer)), optimizer_(std::move(optimizer)) {
    logger_ = CreateOrGetLogger("SolverLogger", verbose);
    optimizer_->SetLogger(logger_);
}

Solution Solver::Solve(const InputData& input_data) {
    auto start_timer = std::chrono::high_resolution_clock::now();

    LOG_DEBUG(logger_, "Building initial routes...");
    RoutePack routes = initializer_->BuildInitialRoutes(input_data);

    LOG_DEBUG(logger_, "Optimizing routes...");
    routes = optimizer_->Optimize(routes, input_data);

    auto end_timer = std::chrono::high_resolution_clock::now();
    double exec_time = std::chrono::duration<double>(end_timer - start_timer).count();

    std::vector<AgentSolution> all_agent_solutions;
    for (const auto& tour : routes.Routes()) {
        AgentSolution sol;
        sol.route = std::vector<uint64_t>(tour->Vertices().begin(), tour->Vertices().end());

        assert(sol.route.back() != 0);
        sol.route.push_back(0);

        sol.total_time = tour->ComputeCost(input_data);
        sol.total_distance = tour->ComputeDistance(input_data);
        sol.total_value = tour->ComputeValue(input_data);
        all_agent_solutions.push_back(sol);
    }

    BenchmarkMetadata meta{};
    optimizer_->EnrichMeta(meta);
    meta.execution_time = exec_time;

    return Solution{all_agent_solutions, meta};
}

BaselineSolver::BaselineSolver(double st, int aon, int max_iter, double time_limit, int verbose)
    : Solver(std::make_unique<InsertionHeuristicInitializer>(),
             std::make_unique<BaselineOptimizer>(st, aon, max_iter, time_limit), verbose) {
}

AnnealingSolver::AnnealingSolver(double st, int aon, int max_iter, double time_limit, double alpha,
                                 int verbose)
    : Solver(std::make_unique<InsertionHeuristicInitializer>(),
             std::make_unique<AnnealingOptimizer>(st, aon, max_iter, time_limit, alpha), verbose) {
}

RebalancingSolver::RebalancingSolver(double st, int aon, int max_iter, double time_limit,
                                     double fairness, int verbose)
    : Solver(std::make_unique<InsertionHeuristicInitializer>(),
             std::make_unique<RebalancingOptimizer>(st, aon, max_iter, time_limit, fairness),
             verbose) {
}
