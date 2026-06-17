#pragma once

#include <optional>

struct SolutionMetrics {
    double value;
    double cost;
    double distance;

    bool operator>(const SolutionMetrics& other) const {
        constexpr double kEps = 1e-9;
        if (value > other.value + kEps) {
            return true;
        }
        if (value < other.value - kEps) {
            return false;
        }
        if (cost < other.cost - kEps) {
            return true;
        }
        if (cost > other.cost + kEps) {
            return false;
        }

        return distance < other.distance - kEps;
    }

    SolutionMetrics operator+(const SolutionMetrics& other) const {
        return {value + other.value, cost + other.cost, distance + other.distance};
    }

    SolutionMetrics operator-(const SolutionMetrics& other) const {
        return {value - other.value, cost - other.cost, distance - other.distance};
    }
};

struct RouteMetricsUpdate {
    SolutionMetrics delta;
    int route1_idx;
    SolutionMetrics route1_metrics;
    std::optional<int> route2_idx = std::nullopt;
    std::optional<SolutionMetrics> route2_metrics = std::nullopt;
};
