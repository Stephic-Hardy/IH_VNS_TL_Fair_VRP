#pragma once

#include "problem_arguments.hpp"

#include <vector>
#include <optional>

class Tour {
    public:
    Tour(size_t n = 0);

    double compute_distance(const InputData& input) const;
    double compute_cost(const InputData& input) const;
    double compute_value(const InputData& input) const;
    void print() const;
    bool validate() const;
    Tour copy() const;
    void invalidate_cache() const;

    std::vector<int> vertices;
private:
    mutable std::optional<double> cached_cost_;
    mutable std::optional<double> cached_value_;
};
