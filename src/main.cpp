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


Tour run_final_2opt(const Tour& initial_tour, const InputData& input_data, const std::vector<int>& new1_to_old0) {
    Tour best_tour = initial_tour.copy();
    bool improved = true;
    double best_cost = best_tour.compute_cost(input_data, new1_to_old0);
    size_t n = best_tour.vertices.size();

    while (improved) {
        improved = false;
        for (size_t i = 1; i < n - 1; ++i) {
            for (size_t j = i + 1; j < n; ++j) {
                Tour neighbor = best_tour.copy();
                // Разворачиваем участок между i и j
                std::reverse(neighbor.vertices.begin() + i, neighbor.vertices.begin() + j + 1);
                neighbor.invalidate_cache();
                
                double current_cost = neighbor.compute_cost(input_data, new1_to_old0);
                if (current_cost < best_cost - 1e-7) {
                    best_cost = current_cost;
                    best_tour = neighbor;
                    improved = true;
                }
            }
        }
    }
    return best_tour;
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
    std::vector<Solution> all_agent_solutions;
    int remaining_points = input_data.points_count - 1; // исключая депо

    std::cout << "Starting multi-agent solver for " << remaining_points << " points..." << std::endl;

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
            if (v != 0) subset_to_visit.push_back(v);
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

        // 4. Начальный тур для VNS
        Tour initial_tour(sub_n);
        initial_tour.vertices.clear();
        initial_tour.vertices.push_back(1); // Депо
        for (int v : subset_to_visit) {
            initial_tour.vertices.push_back(old0_to_new1[v]);
        }

        // 5. Запуск основного алгоритма (VNS + Tabu)
        std::cout << "Agent " << all_agent_solutions.size() << ": optimizing " << subset_to_visit.size() << " points..." << std::endl;
        
        auto [best_vns_tour, _] = VNSTabu::vns_tabu_advanced(
            sub_n, input_data, new1_to_old0, ST, AON, max_iter, time_limit, initial_tour
        );

        // 6. ФИНАЛЬНЫЙ 2-OPT (убирает пересечения и "паутину")
        Tour polished_tour = run_final_2opt(best_vns_tour, input_data, new1_to_old0);

        // 7. Сохранение результата и обновление состояния
        Solution sol;
        sol.route.clear();
        for (int nv : polished_tour.vertices) {
            int ov = new1_to_old0[nv];
            sol.route.push_back(ov);
            if (ov != 0) {
                excluded_points[ov] = true;
                remaining_points--;
            }
        }
        // Гарантируем закрытие маршрута в депо без дублей
        if (sol.route.back() != 0) sol.route.push_back(0);

        sol.solution_size = sol.route.size();
        sol.total_time = static_cast<uint64_t>(polished_tour.compute_cost(input_data, new1_to_old0));
        sol.total_distance = static_cast<uint64_t>(polished_tour.compute_distance(input_data, new1_to_old0));
        sol.total_value = static_cast<uint64_t>(polished_tour.compute_value(input_data, new1_to_old0));

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