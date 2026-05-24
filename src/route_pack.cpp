#include "route_pack.h"

#include <numeric>
#include <cmath>

size_t RoutePack::Size() const {
    return routes_.size();
}

const Route& RoutePack::GetRoute(size_t idx) const {
    return *routes_[idx];
}

const std::vector<std::shared_ptr<Route>>& RoutePack::Routes() const {
    return routes_;
}

Route& RoutePack::MutateRoute(size_t idx) {
    routes_[idx] = std::make_shared<Route>(*routes_[idx]);
    return *routes_[idx];
}

void RoutePack::ReplaceRoute(size_t idx, std::shared_ptr<Route> route) {
    routes_[idx] = route;
}

void RoutePack::AddRoute(Route&& route) {
    routes_.push_back(std::make_shared<Route>(std::move(route)));
}

double RoutePack::ComputeMaxDistance(const InputData& input) const {
    double max_cost = 0;
    for (auto& route : routes_) {
        max_cost = std::max(max_cost, route->ComputeDistance(input));
    }
    return max_cost;
}

double RoutePack::ComputeMinDistance(const InputData& input) const {
    double min_cost = 0;
    for (auto& route : routes_) {
        min_cost = std::min(min_cost, route->ComputeDistance(input));
    }
    return min_cost;
}

bool RoutePack::IsValid(const InputData& input) const {
    for (auto& route : routes_) {
        if (route->Length() - 1 > input.max_load) {
            return false;
        }
        if (route->ComputeCost(input) > input.max_time) {
            return false;
        }
        if (route->ComputeDistance(input) > input.max_distance) {
            return false;
        }
    }
    return true;
}

double RoutePack::ComputeDistance(const InputData& input) const {
    double total = 0;
    for (auto& route : routes_) {
        total += route->ComputeDistance(input);
    }
    return total;
}

double RoutePack::ComputeCost(const InputData& input) const {
    double total = 0;
    for (auto& route : routes_) {
        total += route->ComputeCost(input);
    }
    return total;
}

double RoutePack::ComputeValue(const InputData& input) const {
    double total = 0;
    for (auto& route : routes_) {
        total += route->ComputeValue(input);
    }
    return total;
}

double RoutePack::ComputeDistanceStandardDeviation(const InputData& input) const {
    if (routes_.size() <= 1) {
        return 0.0;
    }

    double sum = 0;
    double sum_of_squares = 0;
    for (const auto& route : routes_) {
        double dist = route->ComputeDistance(input);
        sum += dist;
        sum_of_squares += dist * dist;
    }

    double mean = sum / routes_.size();
    return std::sqrt(std::abs(sum_of_squares / routes_.size() - mean * mean));
}

size_t RoutePack::GetMaxDistanceRouteIdx(const InputData& input) const {
    size_t best_idx = 0;
    double max_d = -1.0;
    for (size_t i = 0; i < routes_.size(); ++i) {
        double d = routes_[i]->ComputeDistance(input);
        if (d > max_d) {
            max_d = d;
            best_idx = i;
        }
    }
    return best_idx;
}

size_t RoutePack::GetMinDistanceRouteIdx(const InputData& input) const {
    size_t best_idx = 0;
    double min_d = std::numeric_limits<double>::max();
    for (size_t i = 0; i < routes_.size(); ++i) {
        double d = routes_[i]->ComputeDistance(input);
        if (d < min_d) {
            min_d = d;
            best_idx = i;
        }
    }
    return best_idx;
}

bool RoutePack::operator==(const RoutePack& route_pack) const {
    if (routes_.size() != route_pack.routes_.size()) {
        return false;
    }
    for (size_t i = 0; i < routes_.size(); i++) {
        if (*routes_[i] != *route_pack.routes_[i]) {
            return false;
        }
    }
    return true;
}

bool RoutePack::operator!=(const RoutePack& route_pack) const {
    return !(*this == route_pack);
}