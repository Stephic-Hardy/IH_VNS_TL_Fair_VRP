#include "post_processing.h"
#include "tour.h"

#include <algorithm>
#include <optional>

namespace {
    /**
     * Tries to reverse a subpath (helps to get rid of self-intersections)
     */
    std::optional<Tour> run_2_opt(const Tour &baseline_tour, const InputData &input_data,
                                  const std::vector<int> &new1_to_old0) {
        double baseline_cost = baseline_tour.compute_cost(input_data, new1_to_old0);

        size_t n = baseline_tour.vertices.size();
        for (size_t i = 1; i < n - 1; ++i) {
            for (size_t j = i + 1; j < n; ++j) {
                Tour neighbor = baseline_tour.copy();
                // Разворачиваем участок между i и j
                std::reverse(neighbor.vertices.begin() + i, neighbor.vertices.begin() + j + 1);
                neighbor.invalidate_cache();

                double current_cost = neighbor.compute_cost(input_data, new1_to_old0);
                if (current_cost < baseline_cost - 1e-7) {
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
        double baseline_cost = baseline_tour.compute_cost(input_data, new1_to_old0);

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

                double current_cost = neighbor.compute_cost(input_data, new1_to_old0);
                if (current_cost < baseline_cost - 1e-7) {
                    return {neighbor};
                }
            }
        }
        return std::nullopt;
    }
}

Tour post_process(const Tour &initial_tour, const InputData &input_data, const std::vector<int> &new1_to_old0) {
    Tour best_tour = initial_tour.copy();
    bool improved = true;

    while (improved) {
        improved = false;

        auto optimization_result = run_2_opt(best_tour, input_data, new1_to_old0);
        if (optimization_result.has_value()) {
            best_tour = optimization_result.value();
            improved = true;
        }

        optimization_result = move_vertex(best_tour, input_data, new1_to_old0);
        if (optimization_result.has_value()) {
            best_tour = optimization_result.value();
            improved = true;
        }
    }
    return best_tour;
}
