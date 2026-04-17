#pragma once

#include "problem_arguments.hpp"

#include <vector>
#include <optional>

class Tour {
private:
    mutable std::optional<double> cached_cost;
    mutable std::optional<double> cached_value;

public:
    std::vector<int> vertices;

    Tour(int n = 0);

    double compute_distance(const InputData& input, const std::vector<int>& new1_to_old0) const;

    double compute_distance(const InputData& input) const;

    double compute_cost(const InputData& input, const std::vector<int>& new1_to_old0) const;
    double compute_value(const InputData& input, const std::vector<int>& new1_to_old0) const;
    void print() const;
    bool validate(int n) const;
    Tour copy() const;
    void invalidate_cache() const;
};
