#include "initializer.h"
#include <iostream>
#include "first_step.hpp"
#include "problem_arguments.hpp"

RoutePack InsertionHeuristicInitializer::BuildInitialRoutes(const InputData &input_data) {
    std::vector<bool> excluded_points(input_data.points_count, false);
    size_t remaining_points = input_data.points_count - 1;

    RoutePack routes;
    while (remaining_points >= input_data.min_load) {
        // Find subset of points for a new route
        FirstStepAnswer fs_ans = DoFirstStep<true>(input_data, excluded_points);

        if (fs_ans.vertexes.size() < static_cast<size_t>(input_data.min_load)) {
            break;
        }

        // Remove zeroes from the found subset
        std::vector<int> subset_to_visit;
        subset_to_visit.reserve(fs_ans.vertexes.size());
        for (int v : fs_ans.vertexes) {
            if (v != 0) {
                subset_to_visit.push_back(v);
            }
        }

        // Construct initial route
        subset_to_visit.insert(subset_to_visit.begin(), 0);
        routes.AddRoute(Route(subset_to_visit));

        // Remove visited vertices
        bool added_any = false;
        for (int v : subset_to_visit) {
            if (!excluded_points[v] && v != 0) {
                excluded_points[v] = true;
                remaining_points--;
                added_any = true;
            }
        }
        if (!added_any) {
            break;
        }
    }

    return routes;
}
