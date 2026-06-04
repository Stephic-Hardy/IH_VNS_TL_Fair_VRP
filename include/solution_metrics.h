#pragma once

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
};
