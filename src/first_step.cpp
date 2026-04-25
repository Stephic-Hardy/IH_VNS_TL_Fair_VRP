#include "first_step.hpp"

#include <algorithm>
#include <iostream>

bool FirstStepAnswer::IsVertexInPath(const uint64_t id) {
    if (id == 0) {
        return false;
    }
    return std::any_of(vertexes.begin(), vertexes.end(),
                       [&](auto v_idx) { return id == v_idx; });
}

void FirstStepAnswer::AddVertex(uint64_t vertex) {
    vertexes.push_back(vertex);
}

template<bool is_time_dependent>
FirstStepAnswer DoFirstStep(const InputData &input, const std::vector<bool>& excluded_points) {
    auto max_dist = input.max_distance;
    auto max_time = input.max_time;
    auto max_load = input.max_load;
    auto min_load = input.min_load;
    auto points_count = input.points_count;

    std::vector<std::vector<FirstStepAnswer>> dp(input.max_load + 2,
                                                 std::vector<FirstStepAnswer>(points_count, FirstStepAnswer{}));
    dp[0][0].value = 0;

    for (size_t cur_load = 0; cur_load <= max_load; ++cur_load) {
        bool find_update_point = false;
        for (size_t j = 0; j < points_count; ++j) {
            if (j != 0 && excluded_points[j]) continue; 

            auto new_best_point_score = FirstStepAnswer::default_value;
            uint64_t new_best_point_time = 0;
            uint64_t new_best_point_dist = 0;
            auto prev_best_point = -1;

            for (size_t i = 0; i < points_count; ++i) {
                if (i == 0 && cur_load != 0) continue;

                if (i != j &&
                    dp[cur_load][i].value != FirstStepAnswer::default_value &&
                    !dp[cur_load][i].IsVertexInPath(j)) {

                    int64_t travel_time;
                    if constexpr (is_time_dependent) {
                        travel_time = input.get_time_dependent_cost(dp[cur_load][i].time, i, j);
                    } else {
                        travel_time = input.time_matrix[0][i][j];
                    }

                    int64_t new_point_score = dp[cur_load][i].value + (j == 0 ? 0 : input.point_scores[j - 1]) - travel_time;
                    auto new_point_time = dp[cur_load][i].time + (j == 0 ? 0 : input.point_service_times[j - 1]) + travel_time;
                    auto new_point_dist = dp[cur_load][i].distance + input.distance_matrix[i][j];

                    if (new_point_score > new_best_point_score && new_point_time <= max_time && new_point_dist <= max_dist) {
                        prev_best_point = i;
                        new_best_point_score = new_point_score;
                        new_best_point_time = new_point_time;
                        new_best_point_dist = new_point_dist;
                    }
                }
            }

            if (prev_best_point != -1) {
                dp[cur_load + 1][j] = dp[cur_load][prev_best_point];
                dp[cur_load + 1][j].time = new_best_point_time;
                dp[cur_load + 1][j].distance = new_best_point_dist;
                dp[cur_load + 1][j].value = new_best_point_score;
                dp[cur_load + 1][j].AddVertex(j);
                find_update_point = true;
            }
        }
        if (!find_update_point) break;
    }

    auto answer = FirstStepAnswer::default_value;
    uint64_t answer_load = 0;
    bool find_answer = false;

    for (auto cur_load = min_load + 1; cur_load <= max_load + 1; ++cur_load) {
        if (answer < dp[cur_load][0].value) {
            answer = dp[cur_load][0].value;
            answer_load = cur_load;
            find_answer = true;
        }
    }
    return find_answer ? dp[answer_load][0] : FirstStepAnswer{};
}

std::ostream &operator<<(std::ostream &os, const FirstStepAnswer &answer) {
    os << "Solution score value: " << answer.value << std::endl;
    os << "Solution path:\n";
    for (auto id: answer.vertexes) {
        os << id << " ";
    }
    os << "\nSolution time: " << answer.time << "\nSolution distance: " << answer.distance << "\nSolution size: "
       << answer.vertexes.size();
    os << std::endl;

    return os;
}

template FirstStepAnswer DoFirstStep<true>(const InputData &input, const std::vector<bool>& excluded_points);
template FirstStepAnswer DoFirstStep<false>(const InputData &input, const std::vector<bool>& excluded_points);