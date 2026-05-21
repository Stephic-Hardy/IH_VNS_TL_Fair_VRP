#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <fstream>
#include <numeric>
#include <iomanip>

#include "route.h"
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

    double min_dist = std::ranges::min(distances);
    double max_dist = std::ranges::max(distances);

    std::cout << "Средняя дистанция:   " << std::fixed << std::setprecision(2) << sum / n <<
        std::endl;
    std::cout << "Коэффициент Джини:   " << std::setprecision(4) << gini << std::endl;
    std::cout << "(Max - Min):   " << std::setprecision(4) << max_dist - min_dist << std::endl;
    std::cout << "(Max - Min) / Min:   " << std::setprecision(4) << (max_dist - min_dist) / min_dist << std::endl;

}

int main(int argc, char* argv[]) {
    if (argc < 7 || (argc > 8 && std::string(argv[7]) == "--benchmark")) {
        std::cerr << "Usage: " << argv[0]
            << " <ST> <AON> <MAX_ITER> <TIME_LIMIT> <INPUT_JSON> <OUTPUT_JSON> [--benchmark]" << std::endl;
        return 1;
    }

    // Arguments parsing
    double ST = std::stod(argv[1]);
    int AON = std::stoi(argv[2]);
    int max_iter = std::stoi(argv[3]);
    int time_limit = std::stoi(argv[4]);
    std::string input_json = argv[5];
    std::string output_json = argv[6];
    bool is_benchmark = (argc >= 8 && std::string(argv[7]) == "--benchmark");

    InputData input_data;
    if (!JsonParser::ParseInputDataFromJson(input_json, input_data)) {
        return 1;
    }

    std::vector<bool> excluded_points(input_data.points_count, false);
    size_t remaining_points = input_data.points_count - 1; // исключая депо

    std::cout << "Starting multi-agent solver for " << remaining_points << " points..." <<
        std::endl;
    auto start_timer = std::chrono::high_resolution_clock::now();

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
        subset_to_visit.insert(subset_to_visit.begin(), 0);
        routes.AddRoute(Route(subset_to_visit));

        // Remove visited vertices
        for (int v : subset_to_visit) {
            if (!excluded_points[v] && v != 0) {
                excluded_points[v] = true;
                remaining_points--;
            }
        }
    }

    std::cout << "\nStarting global fairness optimization across all routes..." << std::endl;
    routes = VNSTabu::VnsTabuGlobal(input_data, ST, AON, max_iter, time_limit, routes);

    // Run the Variable Neighborhood Search Algorithm on each route
    for (size_t route_idx = 0; route_idx < routes.Size(); ++route_idx) {
        std::cout << "Agent " << route_idx
            << ": optimizing " << routes.GetRoute(route_idx).Length()
            << " points..." << std::endl;
        routes = VNSTabu::VnsTabuAdvanced(
            input_data, ST, AON, max_iter, time_limit, routes, route_idx
            );
    }

    std::cout << "Starting global post-processing..." << std::endl;
    PostProcessAllRoutes(routes, input_data);

    auto end_timer = std::chrono::high_resolution_clock::now();
    double exec_time_sec = std::chrono::duration<double>(end_timer - start_timer).count();

    std::vector<Solution> all_agent_solutions;

    for (const auto& tour : routes.Routes()) {
        Solution sol;
        sol.route = std::vector<uint64_t>(tour->Vertices().begin(), tour->Vertices().end());
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
    bool save_success = false;
    if (is_benchmark) {
        BenchmarkMetadata meta{ST, AON, max_iter, time_limit, exec_time_sec};
        save_success = JsonParser::WriteBenchmarkToJsonFile(output_json, all_agent_solutions, meta);
    } else {
        save_success = JsonParser::WriteMultiSolutionToJsonFile(output_json, all_agent_solutions);
    }

    if (save_success) {
        std::cout << "Successfully saved " << all_agent_solutions.size() << " agents to " << output_json << std::endl;
        if (is_benchmark) {
            std::cout << "Execution time: " << exec_time_sec << "s" << std::endl;
        }
    } else {
        std::cerr << "Failed to write output JSON." << std::endl;
        return 1;
    }

    return 0;
}