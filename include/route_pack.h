#pragma once
#include "tour.h"

#include <memory>
#include <vector>

class RoutePack {
public:
    RoutePack& operator=(const RoutePack&) = default;

    void DeepCopyRoute(size_t route_idx);

    void AddRoute(Tour&& route);

    double ComputeDistance(const InputData& input) const;

    double ComputeCost(const InputData& input) const;

    double ComputeValue(const InputData& input) const;

    std::vector<std::shared_ptr<Tour>> routes;
};