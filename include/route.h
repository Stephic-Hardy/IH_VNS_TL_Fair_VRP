#pragma once
#include "problem_arguments.hpp"

#include <functional>
#include <vector>
#include <optional>

class Route {
public:
    Route();

    Route(std::vector<int> vertices);

    Route& operator=(const Route& other) = default;

    size_t Length() const;

    const std::vector<int>& Vertices() const;

    inline void Update(const std::function<void(std::vector<int>&)>& modifier) {
        modifier(vertices_);
        InvalidateCache();
    }

    double ComputeDistance(const InputData& input) const;

    double ComputeCost(const InputData& input) const;

    double ComputeValue(const InputData& input) const;

    std::ostream& operator<<(std::ostream&) const;

    bool operator==(const Route& tour) const;

    bool operator!=(const Route& tour) const;

private:
    void InvalidateCache() const;

    std::vector<int> vertices_;
    mutable std::optional<double> cached_cost_;
    mutable std::optional<double> cached_value_;
    mutable std::optional<double> cached_distance_;
};