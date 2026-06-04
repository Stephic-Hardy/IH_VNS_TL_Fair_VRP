#include "optimizer.h"
#include "post_processing.h"
#include "route_pack.h"
#include "vns_tabu.h"

#include <quill/LogMacros.h>
#include <quill/Logger.h>

void Optimizer::SetLogger(quill::Logger *logger) {
    logger_ = logger;
}

BaselineOptimizer::BaselineOptimizer(double st, int aon, int max_iter, int time_limit)
    : st_(st), aon_(aon), max_iter_(max_iter), time_limit_(time_limit) {
}

void BaselineOptimizer::EnrichMeta(BenchmarkMetadata &meta) {
    meta.st = st_;
    meta.aon = aon_;
    meta.max_iter = max_iter_;
    meta.time_limit = time_limit_;
}

RoutePack BaselineOptimizer::Optimize(RoutePack &routes, const InputData &input_data) {
    RoutePack processed_routes = routes;
    for (size_t route_idx = 0; route_idx < routes.Size(); ++route_idx) {
        LOG_DEBUG(logger_, "Agent {}: optimizing {} points...", route_idx,
                  routes.GetRoute(route_idx).Length());
        routes = VNSTabu::VnsTabuAdvanced(input_data, st_, aon_, max_iter_, time_limit_, routes,
                                          route_idx, logger_);
    }
    PostProcessAllRoutes(routes, input_data);
    return routes;
}

AnnealingOptimizer::AnnealingOptimizer(double st, int aon, int max_iter, int time_limit,
                                       double alpha)
    : st_(st), aon_(aon), max_iter_(max_iter), time_limit_(time_limit), alpha_(alpha) {
}

void AnnealingOptimizer::EnrichMeta(BenchmarkMetadata &meta) {
    meta.st = st_;
    meta.aon = aon_;
    meta.max_iter = max_iter_;
    meta.time_limit = time_limit_;
    meta.alpha = alpha_;
}

RoutePack AnnealingOptimizer::Optimize(RoutePack &routes, const InputData &input_data) {
    LOG_DEBUG(logger_, "Starting global fairness optimization across all routes...");
    routes = VNSTabu::VnsTabuGlobal(input_data, st_, aon_, max_iter_, time_limit_, routes, alpha_,
                                    logger_);

    for (size_t route_idx = 0; route_idx < routes.Size(); ++route_idx) {
        LOG_DEBUG(logger_, "Agent {}: optimizing {} points...", route_idx,
                  routes.GetRoute(route_idx).Length());
        routes = VNSTabu::VnsTabuAdvanced(input_data, st_, aon_, max_iter_, time_limit_, routes,
                                          route_idx, logger_);
    }
    PostProcessAllRoutes(routes, input_data);
    return routes;
}

RebalancingOptimizer::RebalancingOptimizer(double st, int aon, int max_iter, int time_limit,
                                           double fairness)
    : st_(st), aon_(aon), max_iter_(max_iter), time_limit_(time_limit), fairness_(fairness) {
}

void RebalancingOptimizer::EnrichMeta(BenchmarkMetadata &meta) {
    meta.st = st_;
    meta.aon = aon_;
    meta.max_iter = max_iter_;
    meta.time_limit = time_limit_;
    meta.fairness = fairness_;
}

RoutePack RebalancingOptimizer::Optimize(RoutePack &routes, const InputData &input_data) {
    LOG_DEBUG(logger_, "Using Rebalancing Algorithm");
    for (size_t route_idx = 0; route_idx < routes.Size(); ++route_idx) {
        LOG_DEBUG(logger_, "Agent {}: initial optimization...", route_idx + 1);
        routes = VNSTabu::VnsTabuAdvanced(input_data, st_, aon_, max_iter_, time_limit_, routes,
                                          route_idx, logger_);
    }

    LOG_DEBUG(logger_, "\nStep 2: Initial post-processing...");
    PostProcessAllRoutes(routes, input_data);
    LOG_DEBUG(logger_, "\nStep 3: Starting inter-route fairness balancing...");
    BalanceRoutes(routes, input_data, fairness_);
    PostProcessAllRoutes(routes, input_data);
    LOG_DEBUG(logger_, "\nStep 4: Starting final IH_VNS_TL refinement on rebalanced routes...");
    for (size_t i = 0; i < routes.Size(); ++i) {
        LOG_DEBUG(logger_, "Final Polish for Agent {}/{}...", i + 1, routes.Size());
        routes = VNSTabu::VnsTabuAdvanced(input_data, st_, aon_, max_iter_, time_limit_, routes, i,
                                          logger_);
    }

    PostProcessAllRoutes(routes, input_data);
    return routes;
}
