#pragma once

#include "problem_arguments.hpp"

#include <vector>
#include <optional>

class Tour {
public:
    Tour(size_t n = 0);

    Tour& operator=(const Tour& other) = default;

    double ComputeDistance(const InputData& input) const;

    double ComputeCost(const InputData& input) const;

    double ComputeValue(const InputData& input) const;

    void Print() const;

    void InvalidateCache() const;

    Tour Copy() const;

    std::vector<int> vertices;

private:
    mutable std::optional<double> cached_cost_;
    mutable std::optional<double> cached_value_;
};