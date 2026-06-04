#pragma once
#include "route.h"

#include <memory>
#include <vector>

class RoutePack {
public:
    size_t Size() const;

    const Route& GetRoute(size_t idx) const;

    const std::vector<std::shared_ptr<Route>>& Routes() const;

    Route& MutateRoute(size_t idx);

    void ReplaceRoute(size_t idx, std::shared_ptr<Route> route);

    void AddRoute(Route&& route);

    double ComputeMaxDistance(const InputData& input) const;

    double ComputeMinDistance(const InputData& input) const;

    bool IsValid(const InputData& input) const;

    double ComputeDistance(const InputData& input) const;

    double ComputeCost(const InputData& input) const;

    double ComputeValue(const InputData& input) const;

    double ComputeDistanceStandardDeviation(const InputData& input) const;

    size_t GetMaxDistanceRouteIdx(const InputData& input) const;

    size_t GetMinDistanceRouteIdx(const InputData& input) const;

    bool operator==(const RoutePack& route_pack) const;

    bool operator!=(const RoutePack& route_pack) const;

private:
    std::vector<std::shared_ptr<Route>> routes_;
};