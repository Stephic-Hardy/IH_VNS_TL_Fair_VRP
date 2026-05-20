#include "route.h"
#include "problem_arguments.hpp"

#include <algorithm>
#include <random>
#include <chrono>
#include <iostream>

Route::Route() : cached_cost_(std::nullopt), cached_value_(std::nullopt) {
}

Route::Route(std::vector<int> vertices) : vertices_(std::move(vertices)) {
}

size_t Route::Length() const {
    return vertices_.size();
}

const std::vector<int>& Route::Vertices() const {
    return vertices_;
}

double Route::ComputeValue(const InputData& input) const {
    if (cached_value_.has_value()) {
        return cached_value_.value();
    }

    double total_value = 0.0;
    double current_time = 0.0;

    for (size_t i = 1; i < vertices_.size(); ++i) {

        int from = vertices_[i - 1];
        int to = vertices_[i];

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
    int last = vertices_.back();
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

double Route::ComputeCost(const InputData& input) const {
    if (cached_cost_.has_value()) {
        return cached_cost_.value();
    }

    double current_time = 0.0;

    for (size_t i = 1; i < vertices_.size(); ++i) {
        int from = vertices_[i - 1];
        int to = vertices_[i];

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
    int last = vertices_.back();
    int64_t return_time = input.get_time_dependent_cost(
        static_cast<uint64_t>(current_time),
        last,
        0 // депо имеет индекс 0
        );
    current_time += return_time;

    cached_cost_ = current_time;
    return current_time;
}

double Route::ComputeDistance(const InputData& input) const {
    double total_distance = 0.0;

    for (size_t i = 1; i < vertices_.size(); ++i) {
        int from = vertices_[i - 1];
        int to = vertices_[i];

        total_distance += input.distance_matrix[from][to];
    }

    int last = vertices_.back();
    total_distance += input.distance_matrix[last][0];

    return total_distance;
}

std::ostream& Route::operator<<(std::ostream& out) const {
    out << "Tour: ";
    for (int v : vertices_) {
        out << v << " ";
    }
    out << std::endl;
    return out;
}

void Route::InvalidateCache() const {
    cached_cost_ = std::nullopt;
    cached_value_ = std::nullopt;
}

bool Route::operator==(const Route& tour) const {
    return vertices_ == tour.vertices_;
}

bool Route::operator!=(const Route& tour) const {
    return vertices_ != tour.vertices_;
}
