#include "route_pack.h"

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

void RoutePack::AddRoute(Route&& route) {
    routes_.push_back(std::make_shared<Route>(std::move(route)));
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