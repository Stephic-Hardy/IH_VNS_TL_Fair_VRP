#include "post_processing.h"
#include "tour.h"

#include <algorithm>
#include <optional>

#include "problem_arguments.hpp"

namespace {
    /**
     * Tries to reverse a subpath (helps to get rid of self-intersections)
     */
    std::optional<Tour> run_2_opt(const Tour &baseline_tour, const InputData &input_data,
                                  const std::vector<int> &new1_to_old0) {
        double baseline_distance = baseline_tour.compute_distance(input_data, new1_to_old0);

        size_t n = baseline_tour.vertices.size();
        for (size_t i = 1; i < n - 1; ++i) {
            for (size_t j = i + 1; j < n; ++j) {
                Tour neighbor = baseline_tour.copy();
                std::reverse(neighbor.vertices.begin() + i, neighbor.vertices.begin() + j + 1);
                neighbor.invalidate_cache();

                double current_distance = neighbor.compute_distance(input_data, new1_to_old0);
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
    std::optional<Tour> move_vertex(const Tour &baseline_tour, const InputData &input_data,
                                    const std::vector<int> &new1_to_old0) {
        double baseline_distance = baseline_tour.compute_distance(input_data, new1_to_old0);

        size_t n = baseline_tour.vertices.size();
        for (size_t i = 1; i < n - 1; ++i) {
            for (size_t j = 1; j < n - 1; ++j) {
                if (i == j) {
                    continue;
                }
                Tour neighbor = baseline_tour.copy();
                if (i < j) {
                    std::copy(baseline_tour.vertices.begin() + i + 1, baseline_tour.vertices.begin() + j + 1,
                              neighbor.vertices.begin() + i);
                    neighbor.vertices[j] = baseline_tour.vertices[i];
                } else {
                    std::copy(baseline_tour.vertices.begin() + j, baseline_tour.vertices.begin() + i,
                              neighbor.vertices.begin() + j + 1);
                    neighbor.vertices[j] = baseline_tour.vertices[i];
                }
                neighbor.invalidate_cache();

                double current_distance = neighbor.compute_distance(input_data, new1_to_old0);
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
    template<size_t k>
    std::optional<Tour> optimize_k_consecutive(const Tour &baseline_tour, const InputData &input_data,
                                               const std::vector<int> &new1_to_old0) {
        static_assert(k > 1 && k < 10,
                      "It is highly recommended to use k less than 10 due to the algorithm complexity");
        if (baseline_tour.vertices.size() <= k) {
            return std::nullopt;
        }

        Tour best_tour = baseline_tour.copy();

        for (size_t i = 1; i <= best_tour.vertices.size() - k; ++i) {
            size_t prev_old_idx = new1_to_old0[best_tour.vertices[i - 1]];
            size_t next_old_idx = (i + k < best_tour.vertices.size())
                                      ? new1_to_old0[best_tour.vertices[i + k]]
                                      : 0;

            auto compute_segment_distance = [&](const std::vector<int> &window) {
                auto d = static_cast<double>(input_data.distance_matrix[prev_old_idx][new1_to_old0[window[0]]]);

                for (size_t m = 0; m < k - 1; ++m) {
                    d += static_cast<double>(
                        input_data.distance_matrix[new1_to_old0[window[m]]][new1_to_old0[window[m + 1]]]);
                }

                d += static_cast<double>(input_data.distance_matrix[new1_to_old0[window.back()]][next_old_idx]);
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
                best_tour.invalidate_cache();
                return best_tour;
            }
        }
        return std::nullopt;
    }
}

Tour post_process(const Tour &initial_tour, const InputData &input_data, const std::vector<int> &new1_to_old0) {
    Tour best_tour = initial_tour.copy();

    bool improved = false;
    do {
        improved = false;

        auto optimization_result = optimize_k_consecutive<6>(best_tour, input_data, new1_to_old0);
        if (optimization_result.has_value()) {
            best_tour = optimization_result.value();
            improved = true;
            continue;
        }

        optimization_result = run_2_opt(best_tour, input_data, new1_to_old0);
        if (optimization_result.has_value()) {
            best_tour = optimization_result.value();
            improved = true;
            continue;
        }

        optimization_result = move_vertex(best_tour, input_data, new1_to_old0);
        if (optimization_result.has_value()) {
            best_tour = optimization_result.value();
            improved = true;
        }
    } while (improved);
    return best_tour;
}
