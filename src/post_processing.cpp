#include "post_processing.h"
#include "route.h"
#include "problem_arguments.hpp"

#include <algorithm>
#include <optional>

#include "neighborhood.h"


namespace {
/**
 * Tries to reverse a subpath (helps to get rid of self-intersections)
 */
std::optional<RoutePack> Run2Opt(const RoutePack& baseline_tour, size_t route,
                                 const InputData& input_data) {
    auto [neighbour, move] = TwoOptNeighborhood().
        FindBestNeighbor(baseline_tour, input_data, route);
    if (move) {
        return neighbour;
    }
    return std::nullopt;
}

/**
 * Tries to move vertex i to the place j
 * (might reduce the length of the path after removing self-intersection, although I'd rate this as unlikely)
 */
std::optional<RoutePack> MoveVertex(const RoutePack& baseline_tour, size_t route,
                                    const InputData& input_data) {
    auto [neighbour, move] = MoveVertexNeighborhood().FindBestNeighbor(
        baseline_tour, input_data, route);
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
    auto [neighbour, move] = ReorderBlockNeighborhood(k).
        FindBestNeighbor(baseline_tour, input_data, route);
    if (move) {
        return neighbour;
    }
    return std::nullopt;
}
}

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