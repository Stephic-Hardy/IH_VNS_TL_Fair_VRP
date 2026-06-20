#include "post_processing.h"
#include "route.h"
#include "problem_arguments.hpp"
#include "neighborhood.h"

#include <algorithm>
#include <optional>
#include <cmath>
#include <vector>

namespace {
/**
 * Tries to reverse a subpath (helps to get rid of self-intersections)
 */
std::optional<RoutePack> Run2Opt(const RoutePack& baseline_tour, size_t route,
                                 const InputData& input_data) {
    auto [neighbour, move] =
        TwoOptNeighborhood().FindBestNeighbor(baseline_tour, input_data, route);
    if (move) {
        return neighbour;
    }
    return std::nullopt;
}

/**
 * Tries to move vertex i to the place j
 * (might reduce the length of the path after removing self-intersection, although I'd rate this as
 * unlikely)
 */
std::optional<RoutePack> MoveVertex(const RoutePack& baseline_tour, size_t route,
                                    const InputData& input_data) {
    auto [neighbour, move] =
        MoveVertexNeighborhood().FindBestNeighbor(baseline_tour, input_data, route);
    if (move) {
        return neighbour;
    }
    return std::nullopt;
}

/**
 * Tries to reorder each group of k consecutive vertices
 */
template <size_t k>
std::optional<RoutePack> OptimizeKConsecutive(const RoutePack& baseline_tour, size_t route,
                                              const InputData& input_data) {
    static_assert(k > 1 && k < 10,
                  "It is highly recommended to use k less than 10 due to the algorithm complexity");
    auto [neighbour, move] =
        ReorderBlockNeighborhood(k).FindBestNeighbor(baseline_tour, input_data, route);
    if (move) {
        return neighbour;
    }
    return std::nullopt;
}
}  // namespace

RoutePack PostProcessSingleRoute(const RoutePack& initial_tour, size_t route,
                                 const InputData& input_data) {
    RoutePack best_tour = initial_tour;

    bool improved = false;
    do {
        improved = false;

        auto optimization_result = OptimizeKConsecutive<6>(best_tour, route, input_data);
        if (optimization_result.has_value()) {
            best_tour = optimization_result.value();
            improved = true;
            continue;
        }

        optimization_result = Run2Opt(best_tour, route, input_data);
        if (optimization_result.has_value()) {
            best_tour = optimization_result.value();
            improved = true;
            continue;
        }

        optimization_result = MoveVertex(best_tour, route, input_data);
        if (optimization_result.has_value()) {
            best_tour = optimization_result.value();
            improved = true;
        }
    } while (improved);
    return best_tour;
}

void PostProcessAllRoutes(RoutePack& routes, const InputData& input_data) {
    for (size_t route = 0; route < routes.Size(); ++route) {
        routes = PostProcessSingleRoute(routes, route, input_data);
    }
}

void BalanceRoutes(RoutePack& routes, const InputData& input_data, double fairness_importance) {
    bool local_improved = true;
    const double penalty_weight = 1.0 - fairness_importance;

    while (local_improved) {
        local_improved = false;

        size_t max_idx = routes.GetMaxDistanceRouteIdx(input_data);
        size_t min_idx = routes.GetMinDistanceRouteIdx(input_data);
        double dist_max = routes.GetRoute(max_idx).ComputeDistance(input_data);
        double dist_min = routes.GetRoute(min_idx).ComputeDistance(input_data);

        double current_diff = dist_max - dist_min;
        double old_sum = dist_max + dist_min;

        if (max_idx == min_idx || current_diff < 10.0) {
            break;
        }

        auto& r_max = routes.GetRoute(max_idx);
        auto& r_min = routes.GetRoute(min_idx);

        double best_new_diff = current_diff;
        Route best_rmax_cand = r_max;
        Route best_rmin_cand = r_min;

        bool found_move = false;

        for (size_t i = 1; i < r_max.Length(); ++i) {
            int v = r_max.Vertices()[i];
            Route temp_rmax = r_max;
            temp_rmax.Update([i](auto& vertices) { vertices.erase(vertices.begin() + i); });

            double new_max_dist = temp_rmax.ComputeDistance(input_data);

            for (size_t j = 1; j <= r_min.Length(); ++j) {
                Route temp_rmin = r_min;
                temp_rmin.Update(
                    [j, v](auto& vertices) { vertices.insert(vertices.begin() + j, v); });

                if (temp_rmin.Length() - 1 > input_data.max_load) {
                    continue;
                }
                if (temp_rmin.ComputeCost(input_data) > input_data.max_time) {
                    continue;
                }

                double new_min_dist = temp_rmin.ComputeDistance(input_data);
                double new_diff = std::abs(new_max_dist - new_min_dist);
                double new_sum = new_max_dist + new_min_dist;
                double sum_increase = std::max(0.0, new_sum - old_sum);

                if (new_diff < best_new_diff - 1.0 &&
                    std::max(new_max_dist, new_min_dist) < dist_max &&
                    (best_new_diff - new_diff) > (sum_increase * penalty_weight)) {

                    best_new_diff = new_diff;
                    best_rmax_cand = temp_rmax;
                    best_rmin_cand = temp_rmin;
                    found_move = true;
                }
            }
        }
        if (!found_move) {
            for (size_t i = 1; i < r_max.Length(); ++i) {
                int v_max = r_max.Vertices()[i];
                for (size_t j = 1; j < r_min.Length(); ++j) {
                    int v_min = r_min.Vertices()[j];
                    Route temp_rmax = r_max;
                    Route temp_rmin = r_min;

                    temp_rmax.Update([i, v_min](auto& vertices) { vertices[i] = v_min; });
                    temp_rmin.Update([j, v_max](auto& vertices) { vertices[j] = v_max; });

                    if (temp_rmax.ComputeCost(input_data) > input_data.max_time ||
                        temp_rmin.ComputeCost(input_data) > input_data.max_time) {
                        continue;
                    }

                    double n_max_d = temp_rmax.ComputeDistance(input_data);
                    double n_min_d = temp_rmin.ComputeDistance(input_data);
                    double new_diff = std::abs(n_max_d - n_min_d);
                    double new_sum = n_max_d + n_min_d;
                    double sum_increase = std::max(0.0, new_sum - old_sum);

                    if (new_diff < best_new_diff - 1.0 && std::max(n_max_d, n_min_d) < dist_max &&
                        (best_new_diff - new_diff) > (sum_increase * penalty_weight)) {

                        best_new_diff = new_diff;
                        best_rmax_cand = temp_rmax;
                        best_rmin_cand = temp_rmin;
                        found_move = true;
                    }
                }
            }
        }

        if (found_move) {
            routes.ReplaceRoute(max_idx, std::make_shared<Route>(best_rmax_cand));
            routes.ReplaceRoute(min_idx, std::make_shared<Route>(best_rmin_cand));
            routes = PostProcessSingleRoute(routes, max_idx, input_data);
            routes = PostProcessSingleRoute(routes, min_idx, input_data);
            local_improved = true;
        }
    }
}
