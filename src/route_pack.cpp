#include "route_pack.h"

#include "route_pack.h"

// Deep copies only the specific route so we can modify it without affecting other copies
void RoutePack::isolate_route_for_mutation(int route_idx) {
    if (route_idx >= 0 && route_idx < routes.size()) {
        // Create a new Tour object (deep copy) and point the shared_ptr to it
        routes[route_idx] = std::make_shared<Tour>(*routes[route_idx]);
    }
}

double RoutePack::compute_distance(const InputData& input) const {
    double total = 0;
    // For global indices, we use an identity mapping: new index i is old index i
    std::vector<int> identity(input.points_count + 1);
    for(int i=0; i<identity.size(); ++i) identity[i] = i;

    for (auto& route : routes) {
        total += route->compute_distance(input, identity);
    }
    return total;
}

double RoutePack::compute_cost(const InputData& input) const {
    double total = 0;
    std::vector<int> identity(input.points_count + 1);
    for(int i=0; i<identity.size(); ++i) identity[i] = i;

    for (auto& route : routes) {
        total += route->compute_cost(input, identity);
    }
    return total;
}

double RoutePack::compute_value(const InputData& input) const {
    double total = 0;
    std::vector<int> identity(input.points_count + 1);
    for(int i=0; i<identity.size(); ++i) identity[i] = i;

    for (auto& route : routes) {
        total += route->compute_value(input, identity);
    }
    return total;
}
