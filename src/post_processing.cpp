#include "post_processing.h"
#include "tour.h"
#include "problem_arguments.hpp"

#include <algorithm>
#include <optional>
#include <numeric>
#include <cmath>
#include <vector>
#include <iostream>

namespace {
/**
 * Tries to reverse a subpath (helps to get rid of self-intersections)
 */
std::optional<Tour> Run2Opt(const Tour& baseline_tour, const InputData& input_data) {
    double baseline_distance = baseline_tour.ComputeDistance(input_data);

    size_t n = baseline_tour.vertices.size();
    for (size_t i = 1; i < n - 1; ++i) {
        for (size_t j = i + 1; j < n; ++j) {
            Tour neighbor = baseline_tour.Copy();
            std::reverse(neighbor.vertices.begin() + i, neighbor.vertices.begin() + j + 1);
            neighbor.InvalidateCache();

            double current_distance = neighbor.ComputeDistance(input_data);
            if (current_distance < baseline_distance - 1e-7) {
                return {neighbor};
            }
        }
    }
    return std::nullopt;
}

/**
 * Tries to move vertex i to the place j
 * (might reduce the length of the path after removing self-intersection, although I'd rate this as unlikely)
 */
std::optional<Tour> MoveVertex(const Tour& baseline_tour, const InputData& input_data) {
    double baseline_distance = baseline_tour.ComputeDistance(input_data);

    size_t n = baseline_tour.vertices.size();
    for (size_t i = 1; i < n - 1; ++i) {
        for (size_t j = 1; j < n - 1; ++j) {
            if (i == j) {
                continue;
            }
            Tour neighbor = baseline_tour.Copy();
            if (i < j) {
                std::copy(baseline_tour.vertices.begin() + i + 1,
                          baseline_tour.vertices.begin() + j + 1,
                          neighbor.vertices.begin() + i);
                neighbor.vertices[j] = baseline_tour.vertices[i];
            } else {
                std::copy(baseline_tour.vertices.begin() + j, baseline_tour.vertices.begin() + i,
                          neighbor.vertices.begin() + j + 1);
                neighbor.vertices[j] = baseline_tour.vertices[i];
            }
            neighbor.InvalidateCache();

            double current_distance = neighbor.ComputeDistance(input_data);
            if (current_distance < baseline_distance - 1e-7) {
                return {neighbor};
            }
        }
    }
    return std::nullopt;
}

/**
 * Tries to reorder each group of k consecutive vertices
 */
template <size_t k>
std::optional<Tour> OptimizeKConsecutive(const Tour& baseline_tour, const InputData& input_data) {
    static_assert(k > 1 && k < 10,
                  "It is highly recommended to use k less than 10 due to the algorithm complexity");
    if (baseline_tour.vertices.size() <= k) {
        return std::nullopt;
    }

    Tour best_tour = baseline_tour.Copy();

    for (size_t i = 1; i <= best_tour.vertices.size() - k; ++i) {
        size_t prev_idx = best_tour.vertices[i - 1];
        size_t next_idx = (i + k < best_tour.vertices.size())
                              ? best_tour.vertices[i + k]
                              : 0;

        auto compute_segment_distance = [prev_idx, next_idx, &input_data](
            const std::vector<int>& window) {
            auto d = static_cast<double>(input_data.distance_matrix[prev_idx][window[0]]);

            for (size_t m = 0; m < k - 1; ++m) {
                d += static_cast<double>(
                    input_data.distance_matrix[window[m]][window[m + 1]]);
            }

            d += static_cast<double>(input_data.distance_matrix[window.back()][next_idx]);
            return d;
        };

        std::vector<int> current_window(best_tour.vertices.begin() + i,
                                        best_tour.vertices.begin() + i + k);

        double original_dist = compute_segment_distance(current_window);
        double best_local_dist = original_dist;
        std::vector<int> best_permutation = current_window;

        std::vector<int> working_window = current_window;
        std::sort(working_window.begin(), working_window.end());
        bool found_improvement = false;
        do {
            double current_dist = compute_segment_distance(working_window);
            if (current_dist < best_local_dist - 1e-7) {
                best_local_dist = current_dist;
                best_permutation = working_window;
                found_improvement = true;
            }
        } while (std::next_permutation(working_window.begin(), working_window.end()));

        if (found_improvement) {
            for (size_t m = 0; m < k; ++m) {
                best_tour.vertices[i + m] = best_permutation[m];
            }
            best_tour.InvalidateCache();
            return best_tour;
        }
    }
    return std::nullopt;
}
}

Tour PostProcessSingleRoute(const Tour& initial_tour, const InputData& input_data) {
    Tour best_tour = initial_tour.Copy();

    bool improved = false;
    do {
        improved = false;

        auto optimization_result = OptimizeKConsecutive<6>(best_tour, input_data);
        if (optimization_result.has_value()) {
            best_tour = optimization_result.value();
            improved = true;
            continue;
        }

        optimization_result = Run2Opt(best_tour, input_data);
        if (optimization_result.has_value()) {
            best_tour = optimization_result.value();
            improved = true;
            continue;
        }

        optimization_result = MoveVertex(best_tour, input_data);
        if (optimization_result.has_value()) {
            best_tour = optimization_result.value();
            improved = true;
        }
    } while (improved);
    return best_tour;
}

void PostProcessAllRoutes(RoutePack& routes, const InputData& input_data) {
    for (auto& route : routes.routes) {
        route = std::make_shared<Tour>(PostProcessSingleRoute(*route, input_data));
    }
}

void BalanceRoutes(RoutePack& routes, const InputData& input_data, double fairness_importance) {
    bool local_improved = true;
    const double penalty_weight = 1.0 - fairness_importance; 

    while (local_improved) {
        local_improved = false;

        size_t max_idx = 0, min_idx = 0;
        double dist_max = -1.0, dist_min = std::numeric_limits<double>::max();

        for (size_t i = 0; i < routes.routes.size(); ++i) {
            double d = routes.routes[i]->ComputeDistance(input_data);
            if (d > dist_max) { dist_max = d; max_idx = i; }
            if (d < dist_min) { dist_min = d; min_idx = i; }
        }

        if (max_idx == min_idx) break;

        auto& r_max = *routes.routes[max_idx];
        auto& r_min = *routes.routes[min_idx];
        double current_diff = dist_max - dist_min;
        double old_sum = dist_max + dist_min;

        if (current_diff < 10.0) break;

        int best_v_idx = -1;
        size_t best_insert_pos = 0;
        double best_new_diff = current_diff;
        Tour best_rmax_cand = r_max.Copy();
        Tour best_rmin_cand = r_min.Copy();

        bool found_move = false;

        for (size_t i = 1; i < r_max.vertices.size(); ++i) {
            int v = r_max.vertices[i];
            Tour temp_rmax = r_max.Copy();
            temp_rmax.vertices.erase(temp_rmax.vertices.begin() + i);
            temp_rmax.InvalidateCache();
            double new_max_dist = temp_rmax.ComputeDistance(input_data);

            for (size_t j = 1; j <= r_min.vertices.size(); ++j) {
                Tour temp_rmin = r_min.Copy();
                temp_rmin.vertices.insert(temp_rmin.vertices.begin() + j, v);
                temp_rmin.InvalidateCache();

                if (temp_rmin.vertices.size() - 1 > input_data.max_load) continue;
                if (temp_rmin.ComputeCost(input_data) > input_data.max_time) continue;

                double new_min_dist = temp_rmin.ComputeDistance(input_data);
                double new_diff = std::abs(new_max_dist - new_min_dist);
                double new_sum = new_max_dist + new_min_dist;
                double sum_increase = std::max(0.0, new_sum - old_sum);


                if (new_diff < best_new_diff - 1.0 && 
                    std::max(new_max_dist, new_min_dist) < dist_max &&
                    (best_new_diff - new_diff) > (sum_increase * penalty_weight)) {
                    
                    best_new_diff = new_diff;
                    best_v_idx = i;
                    best_insert_pos = j;
                    best_rmax_cand = temp_rmax;
                    best_rmin_cand = temp_rmin;
                    found_move = true;
                }
            }
        }
        if (!found_move) {
            for (size_t i = 1; i < r_max.vertices.size(); ++i) {
                for (size_t j = 1; j < r_min.vertices.size(); ++j) {
                    Tour temp_rmax = r_max.Copy();
                    Tour temp_rmin = r_min.Copy();

                    std::swap(temp_rmax.vertices[i], temp_rmin.vertices[j]);
                    
                    temp_rmax.InvalidateCache();
                    temp_rmin.InvalidateCache();

                    if (temp_rmax.ComputeCost(input_data) > input_data.max_time || 
                        temp_rmin.ComputeCost(input_data) > input_data.max_time) continue;

                    double n_max_d = temp_rmax.ComputeDistance(input_data);
                    double n_min_d = temp_rmin.ComputeDistance(input_data);
                    double new_diff = std::abs(n_max_d - n_min_d);
                    double new_sum = n_max_d + n_min_d;
                    double sum_increase = std::max(0.0, new_sum - old_sum);

                    if (new_diff < best_new_diff - 1.0 && 
                        std::max(n_max_d, n_min_d) < dist_max &&
                        (best_new_diff - new_diff) > (sum_increase * penalty_weight)) {
                        
                        best_new_diff = new_diff;
                        best_rmax_cand = temp_rmax;
                        best_rmin_cand = temp_rmin;
                        best_v_idx = i; 
                        found_move = true;
                    }
                }
            }
        }


        if (found_move) {
            routes.routes[max_idx] = std::make_shared<Tour>(best_rmax_cand);
            routes.routes[min_idx] = std::make_shared<Tour>(best_rmin_cand);
            routes.routes[max_idx] = std::make_shared<Tour>(PostProcessSingleRoute(*routes.routes[max_idx], input_data));
            routes.routes[min_idx] = std::make_shared<Tour>(PostProcessSingleRoute(*routes.routes[min_idx], input_data));

            local_improved = true;
        }
    }
}