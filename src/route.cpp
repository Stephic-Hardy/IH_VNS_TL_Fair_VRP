#include "route.h"
#include "problem_arguments.hpp"

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

double Route::ComputeDistance(const std::vector<int>& vertices, const InputData& input) {
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

double Route::ComputeCost(const std::vector<int>& vertices, const InputData& input) {
    double current_time = 0.0;

    for (size_t i = 1; i < vertices.size(); ++i) {
        int from = vertices[i - 1];
        int to = vertices[i];

        int64_t travel_time = input.GetTimeDependentCost(static_cast<uint64_t>(current_time),
                                                         from,  // используем старые индексы
                                                         to);

        current_time += travel_time;

        if (to != 0) {
            // 0 - это депо в старых индексах
            current_time += input.point_service_times[to - 1];
        }
    }

    // Возврат в депо
    int last = vertices.back();
    int64_t return_time = input.GetTimeDependentCost(static_cast<uint64_t>(current_time), last,
                                                     0  // депо имеет индекс 0
    );
    current_time += return_time;

    return current_time;
}

double Route::ComputeValue(const std::vector<int>& vertices, const InputData& input) {
    double total_value = 0.0;
    double current_time = 0.0;

    for (size_t i = 1; i < vertices.size(); ++i) {

        int from = vertices[i - 1];
        int to = vertices[i];

        if (to != 0) {
            total_value += input.point_scores[to - 1];
        }

        int64_t travel_time =
            input.GetTimeDependentCost(static_cast<uint64_t>(current_time), from, to);

        total_value -= travel_time;

        current_time += travel_time;
        if (to != 0) {
            current_time += input.point_service_times[to - 1];
        }
    }

    // Учитываем возврат в депо
    int last = vertices.back();
    int64_t return_time = input.GetTimeDependentCost(static_cast<uint64_t>(current_time), last, 0);

    // Вычитаем время возврата в депо
    total_value -= return_time;

    return total_value;
}

double Route::ComputeValue(const InputData& input) const {
    if (cached_value_.has_value()) {
        return cached_value_.value();
    }
    cached_value_ = Route::ComputeValue(vertices_, input);
    return cached_value_.value();
}

double Route::ComputeCost(const InputData& input) const {
    if (cached_cost_.has_value()) {
        return cached_cost_.value();
    }
    cached_cost_ = Route::ComputeCost(vertices_, input);
    return cached_cost_.value();
}

double Route::ComputeDistance(const InputData& input) const {
    if (cached_distance_.has_value()) {
        return cached_distance_.value();
    }
    cached_distance_ = Route::ComputeDistance(vertices_, input);
    return cached_distance_.value();
}

void Route::InvalidateCache() const {
    cached_cost_ = std::nullopt;
    cached_value_ = std::nullopt;
    cached_distance_ = std::nullopt;
}

bool Route::operator==(const Route& tour) const {
    return vertices_ == tour.vertices_;
}

bool Route::operator!=(const Route& tour) const {
    return vertices_ != tour.vertices_;
}
