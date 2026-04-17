#include "tour.h"
#include "problem_arguments.hpp"

#include <algorithm>
#include <cassert>
#include <random>
#include <chrono>
#include <iostream>
#include <set>

Tour::Tour(size_t n) : vertices(n), cached_cost_(std::nullopt), cached_value_(std::nullopt) {
    if (n == 0) {
        return;
    }

    for (size_t i = 0; i < n; ++i) {
        vertices[i] = i;
    }
    auto seed = std::chrono::steady_clock::now().time_since_epoch().count();
    std::mt19937 gen(static_cast<unsigned>(seed));
    std::shuffle(vertices.begin() + 1, vertices.end(), gen);
}

double Tour::compute_value(const InputData& input) const {
    if (cached_value_.has_value()) {
        return cached_value_.value();
    }

    double total_value = 0.0;
    double current_time = 0.0;

    for (size_t i = 1; i < vertices.size(); ++i) {

        int from = vertices[i - 1];
        int to = vertices[i];

        if (to != 0) {
            total_value += input.point_scores[to - 1];
        }

        int64_t travel_time = input.get_time_dependent_cost(
            static_cast<uint64_t>(current_time),
            from,
            to
            );

        total_value -= travel_time;

        current_time += travel_time;
        if (to != 0) {
            current_time += input.point_service_times[to - 1];
        }
    }

    // Учитываем возврат в депо
    int last = vertices.back();
    int64_t return_time = input.get_time_dependent_cost(
        static_cast<uint64_t>(current_time),
        last,
        0
        );

    // Вычитаем время возврата в депо
    total_value -= return_time;

    cached_value_ = total_value;
    return total_value;
}

double Tour::compute_cost(const InputData& input) const {
    if (cached_cost_.has_value()) {
        return cached_cost_.value();
    }

    double current_time = 0.0;

    for (size_t i = 1; i < vertices.size(); ++i) {
        int from = vertices[i - 1];
        int to = vertices[i];

        int64_t travel_time = input.get_time_dependent_cost(
            static_cast<uint64_t>(current_time),
            from, // используем старые индексы
            to
            );

        current_time += travel_time;

        if (to != 0) {
            // 0 - это депо в старых индексах
            current_time += input.point_service_times[to - 1];
        }
    }

    // Возврат в депо
    int last = vertices.back();
    int64_t return_time = input.get_time_dependent_cost(
        static_cast<uint64_t>(current_time),
        last,
        0 // депо имеет индекс 0
        );
    current_time += return_time;

    cached_cost_ = current_time;
    return current_time;
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

bool Tour::validate() const {
    if (vertices.empty() || vertices[0] != 0) {
        std::cerr << "Error: Tour must start with depot (index 0)" << std::endl;
        return false;
    }
    std::set<int> unique_vertices(vertices.begin(), vertices.end());
    if (unique_vertices.size() != vertices.size()) {
        std::cerr << "Error: Duplicate vertices in tour" << std::endl;
        return false;
    }
    for (int v : vertices) {
        if (v < 0) {
            // TODO: Wtf is going here? Do we really need this boundary check?
            std::cerr << "Error: Vertex " << v << " has negative index" << std::endl;
            return false;
        }
    }
    return true;
}

void Tour::invalidate_cache() const {
    cached_cost_ = std::nullopt;
    cached_value_ = std::nullopt;
}

Tour Tour::copy() const {
    return Tour(*this);
}