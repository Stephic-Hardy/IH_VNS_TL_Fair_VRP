#include "utils.h"
#include "problem_arguments.hpp"

#include <iomanip>
#include <iostream>
#include <algorithm>
#include <numeric>

void PrintGiniDistance(const Solution &solution) {
    if (solution.agents.empty()) {
        std::cout << "\nНет данных для анализа." << std::endl;
        return;
    }

    size_t n = solution.agents.size();
    std::vector<double> distances;
    for (const auto& sol : solution.agents) {
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

    std::cout << "Средняя дистанция:   " << std::fixed << std::setprecision(2) << sum / n
              << std::endl;
    std::cout << "Коэффициент Джини:   " << std::setprecision(4) << gini << std::endl;
    std::cout << "(Max - Min):   " << std::setprecision(4) << max_dist - min_dist << std::endl;
    std::cout << "(Max - Min) / Min:   " << std::setprecision(4) << (max_dist - min_dist) / min_dist
              << std::endl;
}
