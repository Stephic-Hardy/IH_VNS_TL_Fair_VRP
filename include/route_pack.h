#pragma once
#include "route.h"

#include <memory>
#include <vector>

class RoutePack {
public:
    RoutePack& operator=(const RoutePack&) = default;

    size_t Size() const;

    const Route& GetRoute(size_t idx) const;

    const std::vector<std::shared_ptr<Route>>& Routes() const;

    Route& MutateRoute(size_t idx);

    void AddRoute(Route&& route);

    double ComputeDistance(const InputData& input) const;

    double ComputeCost(const InputData& input) const;

    double ComputeValue(const InputData& input) const;

    bool operator==(const RoutePack& route_pack) const;

    bool operator!=(const RoutePack& route_pack) const;

private:
    std::vector<std::shared_ptr<Route>> routes_;
};