#pragma once
#include "tour.h"

#include <memory>
#include <vector>

struct RoutePack {
    std::vector<std::shared_ptr<Tour>> routes;

    void add_route(Tour&& tour) {
        routes.push_back(std::make_shared<Tour>(std::move(tour)));
    }

    double compute_distance(const InputData& input) const;

    double compute_cost(const InputData& input) const;

    double compute_value(const InputData& input) const;
};