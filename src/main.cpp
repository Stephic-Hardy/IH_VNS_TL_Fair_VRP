#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <fstream>
#include <numeric>
#include <iomanip>
#include <argparse/argparse.hpp>

#include "route.h"
#include "vns_tabu.h"
#include "json_parser.hpp"
#include "post_processing.h"
#include "first_step.hpp"

enum class AlgorithmType { annealing, rebalancing, baseline };

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

    double min_dist = std::ranges::min(distances);
    double max_dist = std::ranges::max(distances);

    std::cout << "Средняя дистанция:   " << std::fixed << std::setprecision(2) << sum / n <<
        std::endl;
    std::cout << "Коэффициент Джини:   " << std::setprecision(4) << gini << std::endl;
    std::cout << "(Max - Min):   " << std::setprecision(4) << max_dist - min_dist << std::endl;
    std::cout << "(Max - Min) / Min:   " << std::setprecision(4) << (max_dist - min_dist) / min_dist
        << std::endl;

}

int main(int argc, char* argv[]) {
    argparse::ArgumentParser program("multi_agent_solver");

    program.add_argument("input-json").help("Path to input JSON file");
    program.add_argument("output-json").help("Path to output JSON file");

    program.add_argument("--st").default_value(0.5).scan<'g', double>().help("ST hyperparameter");
    program.add_argument("--aon").default_value(10).scan<'i', int>().help("AON hyperparameter");
    program.add_argument("--max-iter").default_value(1000).scan<'i', int>().help(
        "Maximum iterations");
    program.add_argument("--time-limit").default_value(60).scan<'i', int>().help(
        "Time limit in seconds");
    program.add_argument("--fairness").default_value(0.5).scan<'g', double>().help(
        "Fairness coefficient (0.0 to 1.0)");

    program.add_argument("-a", "--algorithm")
        .default_value(std::string("annealing"))
        .action([](const std::string& value) {
            static const std::vector<std::string> kChoices = {
                "annealing", "rebalancing", "baseline"
            };
            if (std::find(kChoices.begin(), kChoices.end(), value) != kChoices.end()) {
                return value;
            }
            throw std::runtime_error("Invalid algorithm. Choose 'annealing', 'rebalancing', or 'baseline'.");
        });

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    const double ST = program.get<double>("--st");
    const int AON = program.get<int>("--aon");
    const int max_iter = program.get<int>("--max-iter");
    const int time_limit = program.get<int>("--time-limit");
    const std::string input_json = program.get<std::string>("input-json");
    const std::string output_json = program.get<std::string>("output-json");
    const double fairness = std::clamp(program.get<double>("--fairness"), 0.0, 1.0);

    const std::string alg_name = program.get<std::string>("--algorithm");
    AlgorithmType algorithm;
    if (alg_name == "rebalancing") {
        algorithm = AlgorithmType::rebalancing;
    } else if (alg_name == "baseline") {
        algorithm = AlgorithmType::baseline;
    } else {
        algorithm = AlgorithmType::annealing;
    }

    InputData input_data;
    if (!JsonParser::ParseInputDataFromJson(input_json, input_data)) {
        return 1;
    }

    std::vector<bool> excluded_points(input_data.points_count, false);
    size_t remaining_points = input_data.points_count - 1;

    std::cout << "Starting multi-agent solver for " << remaining_points << " points..." <<
        std::endl;
    auto start_timer = std::chrono::high_resolution_clock::now();

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
        for (int v : subset_to_visit) {
            if (!excluded_points[v] && v != 0) {
                excluded_points[v] = true;
                remaining_points--;
            }
        }
    }

    auto save_and_visualize = [&](const std::string& suffix) {
        std::vector<Solution> current_solutions;
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
            current_solutions.push_back(sol);
        }

        std::string temp_json = output_json;
        size_t dot_pos = temp_json.find_last_of('.');
        if (dot_pos != std::string::npos) {
            temp_json.insert(dot_pos, "_" + suffix);
        } else {
            temp_json += "_" + suffix;
        }

        JsonParser::WriteMultiSolutionToJsonFile(temp_json, current_solutions);
        std::cout << "Saved status to: " << temp_json << std::endl;

        std::string file_name_only = temp_json;
        size_t last_slash = file_name_only.find_last_of("/\\");
        if (last_slash != std::string::npos) {
            file_name_only = file_name_only.substr(last_slash + 1);
        }
        std::string command = "python3 ../scripts/visualize_metrics.py " + file_name_only;
        std::system(command.c_str());
    };

    switch (algorithm) {
        case AlgorithmType::baseline:
            std::cout << "Using Baseline Algorithm" << std::endl;
            for (size_t route_idx = 0; route_idx < routes.Size(); ++route_idx) {
                std::cout << "Agent " << route_idx
                    << ": optimizing " << routes.GetRoute(route_idx).Length()
                    << " points..." << std::endl;
                routes = VNSTabu::VnsTabuAdvanced(
                    input_data, ST, AON, max_iter, time_limit, routes, route_idx
                    );
            }
            PostProcessAllRoutes(routes, input_data);
            break;
        case AlgorithmType::annealing:
            std::cout << "Using Annealing Algorithm" << std::endl;
            std::cout << "\nStarting global fairness optimization across all routes..." <<
                std::endl;
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
            PostProcessAllRoutes(routes, input_data);
            break;
        case AlgorithmType::rebalancing:
            std::cout << "Using Rebalancing Algorithm" << std::endl;
            for (size_t route_idx = 0; route_idx < routes.Size(); ++route_idx) {
                std::cout << "Agent " << route_idx + 1 << ": initial optimization..." << std::endl;
                routes = VNSTabu::VnsTabuAdvanced(
                    input_data, ST, AON, max_iter, time_limit, routes, route_idx
                    );
            }

            std::cout << "\nStep 2: Initial post-processing..." << std::endl;
            PostProcessAllRoutes(routes, input_data);
            save_and_visualize("BEFORE_BALANCE");
            std::cout << "\nStep 3: Starting inter-route fairness balancing..." << std::endl;
            BalanceRoutes(routes, input_data, fairness);
            PostProcessAllRoutes(routes, input_data);
            save_and_visualize("AFTER_BALANCE");

            std::cout << "\nStep 4: Starting final IH_VNS_TL refinement on rebalanced routes..." <<
                std::endl;
            for (size_t i = 0; i < routes.Size(); ++i) {
                std::cout << "Final Polish for Agent " << i + 1 << "/" << routes.Size() << "..." <<
                    std::endl;
                routes = VNSTabu::VnsTabuAdvanced(
                    input_data, ST, AON, max_iter, time_limit, routes, i
                    );
            }

            PostProcessAllRoutes(routes, input_data);
            save_and_visualize("FINAL");
            break;
    }

    auto end_timer = std::chrono::high_resolution_clock::now();
    double exec_time_sec = std::chrono::duration<double>(end_timer - start_timer).count();

    std::vector<Solution> all_agent_solutions;
    for (const auto& tour : routes.Routes()) {
        Solution sol;
        sol.route = std::vector<uint64_t>(tour->Vertices().begin(), tour->Vertices().end());
        if (sol.route.back() != 0)
            sol.route.push_back(0);
        sol.solution_size = sol.route.size();
        sol.total_time = tour->ComputeCost(input_data);
        sol.total_distance = tour->ComputeDistance(input_data);
        sol.total_value = tour->ComputeValue(input_data);
        all_agent_solutions.push_back(sol);
    }

    std::cout << "\n--- FINAL STATISTICS ---" << std::endl;
    PrintGiniDistance(all_agent_solutions);

    // Saving found solution
    bool save_success = false;
    BenchmarkMetadata meta{ST, AON, max_iter, time_limit, exec_time_sec};
    save_success = JsonParser::WriteBenchmarkToJsonFile(output_json, all_agent_solutions, meta);

    if (save_success) {
        std::cout << "Successfully saved " << all_agent_solutions.size() << " agents to " <<
            output_json << std::endl;
        std::cout << "Execution time: " << exec_time_sec << "s" << std::endl;
    } else {
        std::cerr << "Failed to write output JSON." << std::endl;
        return 1;
    }

    return 0;
}