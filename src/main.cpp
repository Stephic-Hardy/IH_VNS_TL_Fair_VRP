#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <fstream>
#include <numeric>
#include <iomanip>

#include "tour.h"
#include "vns_tabu.h"
#include "json_parser.hpp"
#include "post_processing.h"
#include "first_step.hpp"

void PrintGiniDistance(const std::vector<Solution>& solutions) {
    if (solutions.empty()) {
        std::cout << "\nНет данных для анализа." << std::endl;
        return;
    }

    size_t n = solutions.size();
    std::vector<double> distances;
    for (const auto& sol : solutions) {
        distances.push_back(static_cast<double>(sol.total_distance));
    }

    std::sort(distances.begin(), distances.end());

    double sum = std::accumulate(distances.begin(), distances.end(), 0.0);

    if (sum == 0) {
        std::cout << "\nфу, хоть сколько-то проедь" << std::endl;
        return;
    }

    double weighted_sum = 0;
    for (size_t i = 0; i < n; ++i) {
        weighted_sum += (i + 1) * distances[i];
    }

    double gini = (2.0 * weighted_sum) / (n * sum) - (static_cast<double>(n) + 1.0) / n;

    std::cout << "Средняя дистанция:   " << std::fixed << std::setprecision(2) << sum / n <<
        std::endl;
    std::cout << "Коэффициент Джини:   " << std::setprecision(4) << gini << std::endl;

}

int main(int argc, char* argv[]) {
    if (argc != 7) {
        std::cerr << "Usage: " << argv[0]
            << " <ST> <AON> <MAX_ITER> <TIME_LIMIT> <INPUT_JSON> <OUTPUT_JSON>" << std::endl;
        return 1;
    }

    // Arguments parsing
    double ST = std::stod(argv[1]);
    int AON = std::stoi(argv[2]);
    int max_iter = std::stoi(argv[3]);
    int time_limit = std::stoi(argv[4]);
    std::string input_json = argv[5];
    std::string output_json = argv[6];

    InputData input_data;
    if (!JsonParser::ParseInputDataFromJson(input_json, input_data)) {
        return 1;
    }

    std::vector<bool> excluded_points(input_data.points_count, false);
    size_t remaining_points = input_data.points_count - 1; // исключая депо

    std::cout << "Starting multi-agent solver for " << remaining_points << " points..." <<
        std::endl;

    RoutePack routes;

    while (remaining_points >= input_data.min_load) {
        // Find subset of points for a new route
        FirstStepAnswer fs_ans = DoFirstStep<true>(input_data, excluded_points);

        if (fs_ans.vertexes.size() < static_cast<size_t>(input_data.min_load)) {
            break; // Больше не можем собрать валидный маршрут
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
        Tour initial_tour(1);
        for (int v : subset_to_visit) {
            initial_tour.vertices.push_back(v);
        }
        routes.AddRoute(std::move(initial_tour));

        // Run the Variable Neighborhood Search Algorithm
        std::cout << "Agent " << routes.routes.size() << ": optimizing " << subset_to_visit.size()
            << " points..." << std::endl;
        routes = VNSTabu::VnsTabuAdvanced(
            input_data, ST, AON, max_iter, time_limit, routes, routes.routes.size() - 1
            );

        // Remove visited vertices
        for (int v : subset_to_visit) {
            if (!excluded_points[v]) {
                excluded_points[v] = true;
                remaining_points--;
            }
        }
    }

    std::cout << "Starting global post-processing..." << std::endl;
    PostProcessAllRoutes(routes, input_data);

    std::vector<Solution> all_agent_solutions;

    for (const auto& tour : routes.routes) {
        Solution sol;
        sol.route = std::vector<uint64_t>(tour->vertices.begin(), tour->vertices.end());
        if (sol.route.back() != 0) {
            sol.route.push_back(0);
        }

        sol.solution_size = sol.route.size();
        sol.total_time = tour->ComputeCost(input_data);
        sol.total_distance = tour->ComputeDistance(input_data);
        sol.total_value = tour->ComputeValue(input_data);
        all_agent_solutions.push_back(sol);
    }

    PrintGiniDistance(all_agent_solutions);

    // Saving found solution
    if (JsonParser::WriteMultiSolutionToJsonFile(output_json, all_agent_solutions)) {
        std::cout << "Successfully saved " << all_agent_solutions.size() << " agents to " <<
            output_json << std::endl;
    } else {
        std::cerr << "Failed to write output JSON." << std::endl;
        return 1;
    }

    return 0;
}