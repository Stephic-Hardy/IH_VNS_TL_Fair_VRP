#pragma once

#include <cstdint>
#include <vector>
#include <ostream>
#include <unordered_set>
#include <limits>

#include "../utils/problem_arguments.hpp"

struct FirstStepAnswer {

    static constexpr auto default_value = std::numeric_limits<int64_t>::min();

    /// значение целевой функции
    int64_t value = default_value;
    uint64_t distance = 0;
    uint64_t time = 0;
    /// список вершин в порядке обхода в пути
    std::vector<uint64_t > vertexes{0};

    bool IsVertexInPath(uint64_t id);

    void AddVertex(uint64_t vertex);
};

template<bool is_time_dependent>
FirstStepAnswer DoFirstStep(const InputData &input, const std::vector<bool>& excluded_points);

std::ostream &operator<<(std::ostream &os, const FirstStepAnswer &answer);

