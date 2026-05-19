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

// Функция для расчета и вывода коэффициента Джини
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
        std::cout << "\nНулевая дистанция, расчет невозможен." << std::endl;
        return;
    }

    double weighted_sum = 0;
    for (size_t i = 0; i < n; ++i) {
        weighted_sum += (i + 1) * distances[i];
    }

    double gini = (2.0 * weighted_sum) / (n * sum) - (static_cast<double>(n) + 1.0) / n;

    std::cout << "Средняя дистанция:   " << std::fixed << std::setprecision(2) << sum / n << std::endl;
    std::cout << "Коэффициент Джини:   " << std::setprecision(4) << gini << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 7) {
        std::cerr << "Usage: " << argv[0]
            << " <ST> <AON> <MAX_ITER> <TIME_LIMIT> <INPUT_JSON> <OUTPUT_JSON>" << std::endl;
        return 1;
    }

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
    size_t remaining_points = input_data.points_count - 1; 

    std::cout << "Starting multi-agent solver for " << remaining_points << " points..." << std::endl;

    RoutePack routes;

    while (remaining_points >= input_data.min_load) {
        FirstStepAnswer fs_ans = DoFirstStep<true>(input_data, excluded_points);

        if (fs_ans.vertexes.size() < static_cast<size_t>(input_data.min_load)) {
            break; 
        }

        std::vector<int> subset_to_visit;
        subset_to_visit.reserve(fs_ans.vertexes.size());
        for (int v : fs_ans.vertexes) {
            if (v != 0) subset_to_visit.push_back(v);
        }

        Tour initial_tour(1);
        for (int v : subset_to_visit) {
            initial_tour.vertices.push_back(v);
        }
        routes.AddRoute(std::move(initial_tour));

        std::cout << "Agent " << routes.routes.size() << ": initial optimization..." << std::endl;
        routes = VNSTabu::VnsTabuAdvanced(
            input_data, ST, AON, max_iter, time_limit, routes, routes.routes.size() - 1
            );

        for (int v : subset_to_visit) {
            if (!excluded_points[v]) {
                excluded_points[v] = true;
                remaining_points--;
            }
        }
    }
    auto SaveAndVisualize = [&](const std::string& suffix) {
        std::vector<Solution> current_solutions;
        for (const auto& tour : routes.routes) {
            Solution sol;
            sol.route = std::vector<uint64_t>(tour->vertices.begin(), tour->vertices.end());
            if (sol.route.back() != 0) sol.route.push_back(0);
            sol.solution_size = sol.route.size();
            sol.total_time = tour->ComputeCost(input_data);
            sol.total_distance = tour->ComputeDistance(input_data);
            sol.total_value = tour->ComputeValue(input_data);
            current_solutions.push_back(sol);
        }

        std::string temp_json = output_json;
        size_t dot_pos = temp_json.find_last_of('.');
        if (dot_pos != std::string::npos) temp_json.insert(dot_pos, "_" + suffix);
        else temp_json += "_" + suffix;

        JsonParser::WriteMultiSolutionToJsonFile(temp_json, current_solutions);
        std::cout << "Saved status to: " << temp_json << std::endl;

        std::string fileNameOnly = temp_json;
        size_t lastSlash = fileNameOnly.find_last_of("/\\");
        if (lastSlash != std::string::npos) fileNameOnly = fileNameOnly.substr(lastSlash + 1);

        std::string command = "python3 ../scripts/visualize_metrics.py " + fileNameOnly;
        std::system(command.c_str());
    };

    std::cout << "\nStep 2: Initial post-processing..." << std::endl;
    PostProcessAllRoutes(routes, input_data);
    SaveAndVisualize("BEFORE_BALANCE");
    std::cout << "\nStep 3: Starting inter-route fairness balancing..." << std::endl;
    BalanceRoutes(routes, input_data);
    PostProcessAllRoutes(routes, input_data);
    SaveAndVisualize("AFTER_BALANCE");

    std::cout << "\nStep 4: Starting final IH_VNS_TL refinement on rebalanced routes..." << std::endl;
    for (size_t i = 0; i < routes.routes.size(); ++i) {
        std::cout << "Final Polish for Agent " << i + 1 << "/" << routes.routes.size() << "..." << std::endl;
        routes = VNSTabu::VnsTabuAdvanced(
            input_data, ST, AON, max_iter, time_limit, routes, i
        );
    }
    
    PostProcessAllRoutes(routes, input_data); 
    SaveAndVisualize("FINAL");

    std::vector<Solution> all_agent_solutions;
    for (const auto& tour : routes.routes) {
        Solution sol;
        sol.route = std::vector<uint64_t>(tour->vertices.begin(), tour->vertices.end());
        if (sol.route.back() != 0) sol.route.push_back(0);
        sol.solution_size = sol.route.size();
        sol.total_time = tour->ComputeCost(input_data);
        sol.total_distance = tour->ComputeDistance(input_data);
        sol.total_value = tour->ComputeValue(input_data);
        all_agent_solutions.push_back(sol);
    }

    std::cout << "\n--- FINAL STATISTICS ---" << std::endl;
    PrintGiniDistance(all_agent_solutions);

    if (JsonParser::WriteMultiSolutionToJsonFile(output_json, all_agent_solutions)) {
        std::cout << "Successfully saved results to " << output_json << std::endl;
    }

    return 0;
}