#include "tour.h"
#include "problem_arguments.hpp"

#include <algorithm>
#include <random>
#include <chrono>
#include <iostream>
#include <set>

Tour::Tour(int n) : cached_cost(std::nullopt), cached_value(std::nullopt) {
    vertices.resize(n);
    vertices[0] = 1;
    for (int i = 1; i < n; ++i) {
        vertices[i] = i + 1;
    }
    auto seed = std::chrono::steady_clock::now().time_since_epoch().count();
    std::mt19937 gen(static_cast<unsigned>(seed));
    std::shuffle(vertices.begin() + 1, vertices.end(), gen);
}

double Tour::compute_value(const InputData& input, const std::vector<int>& new1_to_old0) const {
    if (cached_value.has_value()) {
        return cached_value.value();
    }

    double total_value = 0.0;
    double current_time = 0.0;

    for (size_t i = 1; i < vertices.size(); ++i) {

        int from_new = vertices[i - 1];
        int to_new = vertices[i];

        int from_old = new1_to_old0[from_new];
        int to_old = new1_to_old0[to_new];

        if (to_old != 0) {
            total_value += input.point_scores[to_old - 1];
        }

        int64_t travel_time = input.get_time_dependent_cost(
            static_cast<uint64_t>(current_time),
            from_old,
            to_old
            );

        total_value -= travel_time;

        current_time += travel_time;
        current_time += input.point_service_times[to_old - 1];

    }

    // Учитываем возврат в депо
    int last_new = vertices.back();
    int last_old = new1_to_old0[last_new];
    int64_t return_time = input.get_time_dependent_cost(
        static_cast<uint64_t>(current_time),
        last_old,
        0
        );

    // Вычитаем время возврата в депо
    total_value -= return_time;

    cached_value = total_value;
    return total_value;
}

double Tour::compute_cost(const InputData& input, const std::vector<int>& new1_to_old0) const {
    if (cached_cost.has_value()) {
        return cached_cost.value();
    }

    double current_time = 0.0;

    for (size_t i = 1; i < vertices.size(); ++i) {
        int from_new = vertices[i - 1];
        int to_new = vertices[i];

        // Преобразуем новые индексы в старые
        int from_old = new1_to_old0[from_new];
        int to_old = new1_to_old0[to_new];

        int64_t travel_time = input.get_time_dependent_cost(
            static_cast<uint64_t>(current_time),
            from_old, // используем старые индексы
            to_old
            );

        current_time += travel_time;

        if (to_old != 0) {
            // 0 - это депо в старых индексах
            current_time += input.point_service_times[to_old - 1];
        }
    }

    // Возврат в депо
    int last_new = vertices.back();
    int last_old = new1_to_old0[last_new];
    int64_t return_time = input.get_time_dependent_cost(
        static_cast<uint64_t>(current_time),
        last_old,
        0 // депо имеет индекс 0
        );
    current_time += return_time;

    cached_cost = current_time;
    return current_time;
}


double Tour::compute_distance(const InputData& input, const std::vector<int>& new1_to_old0) const {
    double total_distance = 0.0;

    for (size_t i = 1; i < vertices.size(); ++i) {
        int from_new = vertices[i - 1];
        int to_new = vertices[i];
        int from_old = new1_to_old0[from_new];
        int to_old = new1_to_old0[to_new];

        total_distance += input.distance_matrix[from_old][to_old];
    }

    int last_new = vertices.back();
    int last_old = new1_to_old0[last_new];
    total_distance += input.distance_matrix[last_old][0];

    return total_distance;
}

double Tour::compute_distance(const InputData& input) const {
    double total_distance = 0.0;

    for (size_t i = 1; i < vertices.size(); ++i) {
        int from = vertices[i - 1];
        int to = vertices[i];

        total_distance += input.distance_matrix[from][to];
    }

    int last = vertices.back();
    total_distance += input.distance_matrix[last][0];

    return total_distance;
}

void Tour::print() const {
    std::cout << "Tour: ";
    for (int v : vertices) {
        std::cout << v << " ";
    }
    std::cout << std::endl;
}

bool Tour::validate(int n) const {
    if (vertices.empty() || vertices[0] != 1) {
        std::cerr << "Error: Tour must start with depot (1)" << std::endl;
        return false;
    }
    std::set<int> unique_vertices(vertices.begin(), vertices.end());
    if (unique_vertices.size() != vertices.size()) {
        std::cerr << "Error: Duplicate vertices in tour" << std::endl;
        return false;
    }
    for (int v : vertices) {
        if (v < 1 || v > n) {
            std::cerr << "Error: Vertex " << v << " out of range [1, " << n << "]" << std::endl;
            return false;
        }
    }
    return true;
}

void Tour::invalidate_cache() const {
    cached_cost = std::nullopt;
    cached_value = std::nullopt;
}

Tour Tour::copy() const {
    return Tour(*this);
}
