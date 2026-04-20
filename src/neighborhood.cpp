#include "neighborhood.h"

#include <functional>

namespace {
constexpr double kEps = 1e-9;
}

std::pair<RoutePack, std::unique_ptr<Move>> Neighborhood::FindBestNeighbor(
    const RoutePack& sol, const InputData& input_data, size_t route) const {
    RoutePack best_neighbor = sol;
    std::unique_ptr<Move> best_move = nullptr;

    double min_cost = best_neighbor.ComputeCost(input_data);
    double best_value = best_neighbor.ComputeValue(input_data);
    double best_distance = best_neighbor.ComputeDistance(input_data);

    std::function eval = [&](const Move& move) {
        RoutePack neighbor = move.Apply(sol);
        double cost = neighbor.ComputeCost(input_data);
        double value = neighbor.ComputeValue(input_data);
        double distance = neighbor.ComputeDistance(input_data);
        if (value > best_value + kEps ||
            (std::abs(value - best_value) < kEps && cost < min_cost - kEps) ||
            (std::abs(value - best_value) < kEps && std::abs(cost - min_cost) < kEps && distance
             < best_distance - kEps)) {
            min_cost = cost;
            best_value = value;
            best_distance = distance;
            best_neighbor = neighbor;
            best_move = move.Clone();
        }
    };

    VisitEachMove(sol, route, eval);

    return {std::move(best_neighbor), std::move(best_move)};
}

std::pair<RoutePack, std::unique_ptr<Move>> Neighborhood::FindBestNeighbor(
    const RoutePack& sol, const InputData& input_data) const {
    RoutePack best_neighbor = sol;
    std::unique_ptr<Move> best_move = nullptr;

    double min_cost = best_neighbor.ComputeCost(input_data);
    double best_value = best_neighbor.ComputeValue(input_data);
    double best_distance = best_neighbor.ComputeDistance(input_data);

    std::function eval = [&](const Move& move) {
        RoutePack neighbor = move.Apply(sol);
        double cost = neighbor.ComputeCost(input_data);
        double value = neighbor.ComputeValue(input_data);
        double distance = neighbor.ComputeDistance(input_data);
        if (value > best_value + kEps ||
            (std::abs(value - best_value) < kEps && cost < min_cost - kEps) ||
            (std::abs(value - best_value) < kEps && std::abs(cost - min_cost) < kEps && distance
             < best_distance - kEps)) {
            min_cost = cost;
            best_value = value;
            best_distance = distance;
            best_neighbor = neighbor;
            best_move = move.Clone();
        }
    };

    VisitEachMove(sol, eval);

    return {std::move(best_neighbor), std::move(best_move)};
}

void Neighborhood::VisitEachMove(const RoutePack& sol,
                                 std::function<void(const Move&)> evaluate) const {
    std::vector<std::unique_ptr<Move>> moves;
    for (size_t route = 0; route < sol.routes.size(); ++route) {
        VisitEachMove(sol, route, evaluate);
    }
}

void RelocateNeighborhood::VisitEachMove(const RoutePack& sol, size_t route,
                                         std::function<void(const Move&)> evaluator) const {
    std::vector<std::unique_ptr<Move>> moves;
    size_t n = sol.routes[route]->vertices.size();
    for (size_t from = 1; from < n - 1; ++from) {
        RemoveInsertMove move(route, from);
        evaluator(move);
    }
}


void SwapAdjNeighborhood::VisitEachMove(const RoutePack& sol, size_t route,
                                        std::function<void(const Move&)> evaluator) const {
    size_t n = sol.routes[route]->vertices.size();
    for (size_t i = 1; i < n - 1; ++i) {
        SwapMove move(route, i, i + 1);
        evaluator(move);
    }
}


void
SwapNeighborhood::VisitEachMove(const RoutePack& sol, size_t route,
                                std::function<void(const Move&)> evaluator) const {
    size_t n = sol.routes[route]->vertices.size();
    for (size_t i = 1; i < n; ++i) {
        for (size_t j = i + 1; j < n; ++j) {
            SwapMove move(route, i, j);
            evaluator(move);
        }
    }
}

void
TwoOptNeighborhood::VisitEachMove(const RoutePack& sol, size_t route,
                                  std::function<void(const Move&)> evaluator) const {
    size_t n = sol.routes[route]->vertices.size();
    for (size_t i = 1; i < n - 2; ++i) {
        for (size_t j = i + 2; j < n; ++j) {
            TwoOptMove move(route, i, j);
            evaluator(move);
        }
    }
}

BlockMoveForwardNeighborhood::BlockMoveForwardNeighborhood(size_t block_size) : k_(block_size) {
}

void
BlockMoveForwardNeighborhood::VisitEachMove(const RoutePack& sol, size_t route,
                                            std::function<void(const Move&)> evaluator) const {
    size_t n = sol.routes[route]->vertices.size();
    for (size_t i = 2; i < n - k_; ++i) {
        BlockRelocateMove move(route, i, k_, i + 1);
        evaluator(move);
    }
}

BlockMoveBackwardNeighborhood::BlockMoveBackwardNeighborhood(size_t block_size) : k_(block_size) {
}

void
BlockMoveBackwardNeighborhood::VisitEachMove(const RoutePack& sol, size_t route,
                                             std::function<void(const Move&)> evaluator) const {
    size_t n = sol.routes[route]->vertices.size();
    for (size_t i = k_ + 1; i < n; ++i) {
        size_t start_pos = i - k_ + 1;
        BlockRelocateMove move(route, start_pos, k_, start_pos - 1);
        evaluator(move);
    }
}