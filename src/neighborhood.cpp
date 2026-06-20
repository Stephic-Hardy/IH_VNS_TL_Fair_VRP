#include "neighborhood.h"
#include "solution_metrics.h"

#include <algorithm>
#include <cassert>
#include <functional>
#include <vector>

namespace {
constexpr double kEps = 1e-9;
}

std::pair<RoutePack, std::unique_ptr<Move>> Neighborhood::FindBestNeighbor(
    const RoutePack& sol, const InputData& input_data, size_t route) const {

    RoutePack best_neighbor = sol;
    std::unique_ptr<Move> best_move = nullptr;

    SolutionMetrics base_metrics = sol.GetRoute(route).ComputeMetrics(input_data);
    SolutionMetrics best_metrics = base_metrics;

    auto eval = [&](const Move& move) {
        RouteMetricsUpdate update = move.EvaluateDelta(sol, input_data);
        assert(update.route1_idx == static_cast<int>(route) && !update.route2_idx.has_value() &&
               !update.route2_metrics.has_value());

        SolutionMetrics neighbor_metrics = update.route1_metrics;

        if (neighbor_metrics > best_metrics) {
            best_metrics = neighbor_metrics;
            best_move = move.Clone();
        }
    };

    VisitEachMove(sol, route, eval);
    if (best_move) {
        best_neighbor = best_move->Apply(sol);
    }
    return {std::move(best_neighbor), std::move(best_move)};
}

std::pair<RoutePack, std::unique_ptr<Move>> Neighborhood::FindBestNeighbor(
    const RoutePack& sol, const InputData& input_data,
    std::function<double(const std::vector<SolutionMetrics>&)> penalty) const {

    RoutePack best_neighbor = sol;
    std::unique_ptr<Move> best_move = nullptr;

    std::vector<SolutionMetrics> base_route_metrics;
    base_route_metrics.reserve(sol.Size());
    for (size_t i = 0; i < sol.Size(); ++i) {
        base_route_metrics.push_back(sol.GetRoute(i).ComputeMetrics(input_data));
    }

    double best_penalty = penalty(base_route_metrics);

    auto eval = [&](const Move& move) {
        RouteMetricsUpdate update = move.EvaluateDelta(sol, input_data);

        SolutionMetrics old_metrics1 = base_route_metrics[update.route1_idx];
        base_route_metrics[update.route1_idx] = update.route1_metrics;
        SolutionMetrics old_metrics2;
        if (update.route2_idx.has_value()) {
            old_metrics2 = base_route_metrics[update.route2_idx.value()];
            base_route_metrics[update.route2_idx.value()] = update.route2_metrics.value();
        }

        double new_penalty = penalty(base_route_metrics);

        base_route_metrics[update.route1_idx] = old_metrics1;
        if (update.route2_idx.has_value()) {
            base_route_metrics[update.route2_idx.value()] = old_metrics2;
        }

        if (new_penalty < best_penalty - kEps) {
            best_penalty = new_penalty;
            best_move = move.Clone();
        }
    };

    VisitEachMove(sol, eval);
    if (best_move) {
        best_neighbor = best_move->Apply(sol);
    }
    return {std::move(best_neighbor), std::move(best_move)};
}

void Neighborhood::VisitEachMove(const RoutePack& sol,
                                 std::function<void(const Move&)> evaluate) const {
    for (size_t route = 0; route < sol.Size(); ++route) {
        VisitEachMove(sol, route, evaluate);
    }
}

void RemovePushBackNeighborhood::VisitEachMove(const RoutePack& sol, size_t route,
                                               std::function<void(const Move&)> evaluator) const {
    size_t n = sol.GetRoute(route).Length();
    for (size_t from = 1; from < n - 1; ++from) {
        RemoveInsertMove move(route, from, n - 1);
        evaluator(move);
    }
}

void MoveVertexNeighborhood::VisitEachMove(const RoutePack& sol, size_t route,
                                           std::function<void(const Move&)> evaluator) const {
    size_t n = sol.GetRoute(route).Length();
    for (size_t from = 1; from < n - 1; ++from) {
        for (size_t to = 1; to < n - 1; ++to) {
            if (to == from) {
                continue;
            }
            RemoveInsertMove move(route, from, to);
            evaluator(move);
        }
    }
}

void SwapAdjNeighborhood::VisitEachMove(const RoutePack& sol, size_t route,
                                        std::function<void(const Move&)> evaluator) const {
    size_t n = sol.GetRoute(route).Length();
    for (size_t i = 1; i < n - 1; ++i) {
        SwapMove move(route, i, i + 1);
        evaluator(move);
    }
}

void SwapNeighborhood::VisitEachMove(const RoutePack& sol, size_t route,
                                     std::function<void(const Move&)> evaluator) const {
    size_t n = sol.GetRoute(route).Length();
    for (size_t i = 1; i < n; ++i) {
        for (size_t j = i + 1; j < n; ++j) {
            SwapMove move(route, i, j);
            evaluator(move);
        }
    }
}

void TwoOptNeighborhood::VisitEachMove(const RoutePack& sol, size_t route,
                                       std::function<void(const Move&)> evaluator) const {
    size_t n = sol.GetRoute(route).Length();
    if (n < 4) {
        return;
    }
    for (size_t i = 1; i < n - 2; ++i) {
        for (size_t j = i + 2; j < n; ++j) {
            TwoOptMove move(route, i, j);
            evaluator(move);
        }
    }
}

BlockMoveForwardNeighborhood::BlockMoveForwardNeighborhood(size_t block_size) : k_(block_size) {
}

void BlockMoveForwardNeighborhood::VisitEachMove(const RoutePack& sol, size_t route,
                                                 std::function<void(const Move&)> evaluator) const {
    size_t n = sol.GetRoute(route).Length();
    if (n <= k_) {
        return;
    }
    for (size_t i = 1; i < n - k_; ++i) {
        BlockRelocateMove move(route, i, k_, i + 1);
        evaluator(move);
    }
}

BlockMoveBackwardNeighborhood::BlockMoveBackwardNeighborhood(size_t block_size) : k_(block_size) {
}

void BlockMoveBackwardNeighborhood::VisitEachMove(
    const RoutePack& sol, size_t route, std::function<void(const Move&)> evaluator) const {
    size_t n = sol.GetRoute(route).Length();
    if (n <= k_) {
        return;
    }
    for (size_t i = 2; i <= n - k_; ++i) {
        BlockRelocateMove move(route, i, k_, i - 1);
        evaluator(move);
    }
}

ReorderBlockNeighborhood::ReorderBlockNeighborhood(size_t k) : k_(k) {
}

void ReorderBlockNeighborhood::VisitEachMove(const RoutePack& sol, size_t route,
                                             std::function<void(const Move&)> evaluator) const {
    size_t n = sol.GetRoute(route).Length();
    if (n <= k_) {
        return;
    }

    for (size_t i = 1; i <= n - k_; ++i) {
        std::vector window(sol.GetRoute(route).Vertices().begin() + i,
                           sol.GetRoute(route).Vertices().begin() + i + k_);

        std::ranges::sort(window);
        do {
            ReorderBlockMove move(route, i, window);
            evaluator(move);
        } while (std::ranges::next_permutation(window).found);
    }
}

void InterRelocateNeighborhood::VisitEachMove(const RoutePack& sol,
                                              std::function<void(const Move&)> evaluator) const {
    size_t num_routes = sol.Size();

    for (size_t r1 = 0; r1 < num_routes; ++r1) {
        for (size_t r2 = 0; r2 < num_routes; ++r2) {
            if (r1 == r2) {
                continue;
            }
            size_t n1 = sol.GetRoute(r1).Length();
            size_t n2 = sol.GetRoute(r2).Length();
            if (n1 <= 1) {
                continue;
            }

            for (size_t i = 1; i < n1; ++i) {
                for (size_t j = 1; j <= n2; ++j) {
                    InterRelocateMove move(r1, r2, i, j);
                    evaluator(move);
                }
            }
        }
    }
}

void InterSwapNeighborhood::VisitEachMove(const RoutePack& sol,
                                          std::function<void(const Move&)> evaluator) const {
    size_t num_routes = sol.Size();

    for (size_t r1 = 0; r1 < num_routes; ++r1) {
        for (size_t r2 = r1 + 1; r2 < num_routes; ++r2) {
            if (r1 == r2) {
                continue;
            }

            size_t n1 = sol.GetRoute(r1).Length();
            size_t n2 = sol.GetRoute(r2).Length();

            for (size_t i = 1; i < n1; ++i) {
                for (size_t j = 1; j < n2; ++j) {
                    InterSwapMove move(r1, r2, i, j);
                    evaluator(move);
                }
            }
        }
    }
}

CrossExchangeNeighborhood::CrossExchangeNeighborhood(size_t k) : k_(k) {
}

void CrossExchangeNeighborhood::VisitEachMove(const RoutePack& sol,
                                              std::function<void(const Move&)> evaluator) const {
    size_t num_routes = sol.Size();

    for (size_t r1 = 0; r1 < num_routes; ++r1) {
        for (size_t r2 = r1 + 1; r2 < num_routes; ++r2) {
            if (r1 == r2) {
                continue;
            }

            size_t n1 = sol.GetRoute(r1).Length();
            size_t n2 = sol.GetRoute(r2).Length();

            for (size_t s1 = 1; s1 < n1; ++s1) {
                for (size_t len1 = 1; len1 <= k_ && s1 + len1 <= n1; ++len1) {
                    for (size_t s2 = 1; s2 < n2; ++s2) {
                        for (size_t len2 = 1; len2 <= k_ && s2 + len2 <= n2; ++len2) {
                            CrossExchangeMove move(r1, r2, s1, len1, s2, len2);
                            evaluator(move);
                        }
                    }
                }
            }
        }
    }
}

void TwoOptStarNeighborhood::VisitEachMove(const RoutePack& sol,
                                           std::function<void(const Move&)> evaluator) const {
    size_t num_routes = sol.Size();

    for (size_t r1 = 0; r1 < num_routes; ++r1) {
        for (size_t r2 = r1 + 1; r2 < num_routes; ++r2) {
            if (r1 == r2) {
                continue;
            }

            size_t n1 = sol.GetRoute(r1).Length();
            size_t n2 = sol.GetRoute(r2).Length();

            for (size_t e1 = 1; e1 + 1 < n1; ++e1) {
                for (size_t e2 = 1; e2 + 1 < n2; ++e2) {
                    TwoOptStarMove move(r1, r2, e1, e2);
                    evaluator(move);
                }
            }
        }
    }
}
