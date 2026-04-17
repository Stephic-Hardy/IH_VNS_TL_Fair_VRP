#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <map>
#include <fstream>

#include "tour.h"
#include "vns_tabu.h"
#include "../utils/json_parser.hpp"
#include "data_adapter.h"
#include "first_step.hpp"
#include <numeric>
#include <algorithm>
#include <iomanip>

#include "post_processing.h"

void print_gini_distance(const std::vector<Solution>& solutions) {
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


    std::cout << "Средняя дистанция:   " << std::fixed << std::setprecision(2) << sum / n << std::endl;
    std::cout << "Коэффициент Джини:   " << std::setprecision(4) << gini << std::endl;


}

int main(int argc, char* argv[]) {
    if (argc != 7) {
        std::cerr << "Usage: " << argv[0] 
                  << " <ST> <AON> <MAX_ITER> <TIME_LIMIT> <INPUT_JSON> <OUTPUT_JSON>" << std::endl;
        return 1;
    }

    // Чтение аргументов
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
    int remaining_points = input_data.points_count - 1; // исключая депо

    std::cout << "Starting multi-agent solver for " << remaining_points << " points..." << std::endl;

    RoutePack routes;

    while (remaining_points >= input_data.min_load) {
        // 1. Получаем начальное подмножество точек через DP (FirstStep)
        // Передаем excluded_points, чтобы не брать уже посещенные точки
        FirstStepAnswer fs_ans = DoFirstStep<true>(input_data, excluded_points);
        
        if (fs_ans.vertexes.size() < static_cast<size_t>(input_data.min_load)) {
            break; // Больше не можем собрать валидный маршрут
        }

        // 2. Подготовка уникальных точек (чистим от лишних нулей FirstStep)
        std::vector<int> subset_to_visit;
        for (int v : fs_ans.vertexes) {
            if (v != 0) {
                subset_to_visit.push_back(v);
            }
        }

        // 3. Создаем маппинг индексов (Депо всегда 1 в локальной нумерации)
        size_t sub_n = subset_to_visit.size() + 1;
        std::vector<int> new1_to_old0(sub_n + 1);
        std::map<int, int> old0_to_new1;

        new1_to_old0[1] = 0; 
        old0_to_new1[0] = 1;
        for (size_t i = 0; i < subset_to_visit.size(); ++i) {
            int old_v = subset_to_visit[i];
            new1_to_old0[i + 2] = old_v;
            old0_to_new1[old_v] = i + 2;
        }

        // Construct initial route
        Tour initial_tour(sub_n);
        initial_tour.vertices.clear();
        initial_tour.vertices.push_back(1); // Депо
        for (int v : subset_to_visit) {
            initial_tour.vertices.push_back(old0_to_new1[v]);
        }

        // Run the Variable Neighborhood Search Algorithm
        std::cout << "Agent " << routes.routes.size() << ": optimizing " << subset_to_visit.size() << " points..." << std::endl;
        auto [best_vns_tour, _] = VNSTabu::vns_tabu_advanced(
            sub_n, input_data, new1_to_old0, ST, AON, max_iter, time_limit, initial_tour
        );

        // Translate route indexing back
        Tour global_tour(best_vns_tour.vertices.size());
        global_tour.vertices.clear();
        for (int local_v : best_vns_tour.vertices) {
            int global_v = new1_to_old0[local_v];
            global_tour.vertices.push_back(global_v);

            if (global_v != 0) {
                excluded_points[global_v] = true;
                remaining_points--;
            }
        }

        routes.add_route(std::move(global_tour));
    }

    std::cout << "Starting global post-processing..." << std::endl;
    PostProcessAllRoutes(routes, input_data);

    std::vector<Solution> all_agent_solutions;
    std::vector<int> identity(input_data.points_count + 1);
    for (size_t i = 0; i < identity.size(); ++i) {
        identity[i] = i;
    }

    for (const auto& tour : routes.routes) {
        Solution sol;
        sol.route = std::vector<uint64_t>(tour->vertices.begin(), tour->vertices.end());
        if (sol.route.back() != 0) {
            sol.route.push_back(0);
        }

        sol.solution_size = sol.route.size();
        sol.total_time = tour->compute_cost(input_data, identity);
        sol.total_distance = tour->compute_distance(input_data, identity);
        sol.total_value = tour->compute_value(input_data, identity);
        all_agent_solutions.push_back(sol);
    }

    print_gini_distance(all_agent_solutions);

    // 8. Запись итогового JSON (в формате массива объектов)
    if (JsonParser::WriteMultiSolutionToJsonFile(output_json, all_agent_solutions)) {
        std::cout << "Successfully saved " << all_agent_solutions.size() << " agents to " << output_json << std::endl;
    } else {
        std::cerr << "Failed to write output JSON." << std::endl;
        return 1;
    }

    return 0;
}