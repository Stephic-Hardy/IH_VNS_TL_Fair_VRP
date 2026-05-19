#pragma once

#include "route_pack.h"
#include "tour.h"

void PostProcessAllRoutes(RoutePack& pack, const InputData& input_data);
void BalanceRoutes(RoutePack& routes, const InputData& input_data, double fairness_importance);
