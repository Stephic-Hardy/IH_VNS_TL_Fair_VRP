#pragma once
#include "problem_arguments.hpp"
#include "solution_metrics.h"

#include <functional>
#include <vector>
#include <optional>

class Route {
public:
    Route();

    Route(std::vector<int> vertices);

    size_t Length() const;

    const std::vector<int>& Vertices() const;

    inline void Update(const std::function<void(std::vector<int>&)>& modifier) {
        modifier(vertices_);
        InvalidateCache();
    }

    SolutionMetrics ComputeMetrics(const InputData& input) const;
    
    double ComputeDistance(const InputData& input) const; // TODO: make them private, leave only ComputeMetrics

    double ComputeCost(const InputData& input) const;

    double ComputeValue(const InputData& input) const;
    
    static SolutionMetrics ComputeMetrics(const std::vector<int>& vertices, const InputData& input);
    
    static double ComputeDistance(const std::vector<int>& vertices, const InputData& input);

    static double ComputeCost(const std::vector<int>& vertices, const InputData& input);

    static double ComputeValue(const std::vector<int>& vertices, const InputData& input);

    bool operator==(const Route& tour) const;

    bool operator!=(const Route& tour) const;

private:
    void InvalidateCache() const;

    std::vector<int> vertices_;
    mutable std::optional<double> cached_cost_;
    mutable std::optional<double> cached_value_;
    mutable std::optional<double> cached_distance_;
};
