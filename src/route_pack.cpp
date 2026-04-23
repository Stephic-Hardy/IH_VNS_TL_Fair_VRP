#include "route_pack.h"

void RoutePack::DeepCopyRoute(size_t route_idx) {
    routes[route_idx] = std::make_shared<Tour>(*routes[route_idx]);
}

void RoutePack::AddRoute(Tour&& route) {
    routes.push_back(std::make_shared<Tour>(std::move(route)));
}

double RoutePack::ComputeDistance(const InputData& input) const {
    double total = 0;
    for (auto& route : routes) {
        total += route->ComputeDistance(input);
    }
    return total;
}

double RoutePack::ComputeCost(const InputData& input) const {
    double total = 0;
    for (auto& route : routes) {
        total += route->ComputeCost(input);
    }
    return total;
}

double RoutePack::ComputeValue(const InputData& input) const {
    double total = 0;
    for (auto& route : routes) {
        total += route->ComputeValue(input);
    }
    return total;
}