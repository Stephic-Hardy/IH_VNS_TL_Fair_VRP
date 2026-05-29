#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <argparse/argparse.hpp>

#include "problem_arguments.hpp"
#include "solver.h"
#include "json_parser.hpp"
#include "utils.h"

enum class AlgorithmType { annealing, rebalancing, baseline };

int main(int argc, char* argv[]) {
    argparse::ArgumentParser program("multi_agent_solver");

    program.add_argument("input-json").help("Path to input JSON file");
    program.add_argument("output-json").help("Path to output JSON file");

    program.add_argument("--st").default_value(0.5).scan<'g', double>().help("ST hyperparameter");
    program.add_argument("--aon").default_value(10).scan<'i', int>().help("AON hyperparameter");
    program.add_argument("--max-iter")
        .default_value(1000)
        .scan<'i', int>()
        .help("Maximum iterations");
    program.add_argument("--time-limit")
        .default_value(60)
        .scan<'i', int>()
        .help("Time limit in seconds");
    program.add_argument("--fairness")
        .default_value(0.5)
        .scan<'g', double>()
        .help("Fairness coefficient (0.0 to 1.0)");
    program.add_argument("--alpha")
        .default_value(0.4)
        .scan<'g', double>()
        .help("Alpha coefficient in the penalty function of the annealing algorithm");

    program.add_argument("-a", "--algorithm")
        .default_value(std::string("annealing"))
        .action([](const std::string& value) {
            static const std::vector<std::string> kChoices = {"annealing", "rebalancing",
                                                              "baseline"};
            if (std::find(kChoices.begin(), kChoices.end(), value) != kChoices.end()) {
                return value;
            }
            throw std::runtime_error(
                "Invalid algorithm. Choose 'annealing', 'rebalancing', or 'baseline'.");
        });

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    const double st = program.get<double>("--st");
    const int aon = program.get<int>("--aon");
    const int max_iter = program.get<int>("--max-iter");
    const int time_limit = program.get<int>("--time-limit");
    const std::string input_json = program.get<std::string>("input-json");
    const std::string output_json = program.get<std::string>("output-json");
    const double fairness = std::clamp(program.get<double>("--fairness"), 0.0, 1.0);
    const double alpha = program.get<double>("--alpha");

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
    
    auto start_timer = std::chrono::high_resolution_clock::now();
    
    Solution solution;
    switch (algorithm) {
        case AlgorithmType::baseline:
            solution = BaselineSolver(st, aon, max_iter, time_limit).Solve(input_data);
            break;
        case AlgorithmType::annealing:
            solution = AnnealingSolver(st, aon, max_iter, time_limit, alpha).Solve(input_data);
            break;
        case AlgorithmType::rebalancing:
            solution = RebalancingSolver(st, aon, max_iter, time_limit, fairness).Solve(input_data);
            break;
    }

    auto end_timer = std::chrono::high_resolution_clock::now();
    double exec_time_sec = std::chrono::duration<double>(end_timer - start_timer).count();

    std::cout << "\n--- FINAL STATISTICS ---" << std::endl;
    PrintGiniDistance(solution);

    // Saving found solution
    bool save_success = false;
    BenchmarkMetadata meta{st, aon, max_iter, time_limit, exec_time_sec};
    save_success = JsonParser::WriteBenchmarkToJsonFile(output_json, solution, meta);

    if (save_success) {
        std::cout << "Successfully saved " << solution.agents.size() << " agents to "
                  << output_json << std::endl;
        std::cout << "Execution time: " << exec_time_sec << "s" << std::endl;
    } else {
        std::cerr << "Failed to write output JSON." << std::endl;
        return 1;
    }

    return 0;
}
