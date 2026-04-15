#pragma once
#include <vector>

#include "tour.h"

Tour post_process(const Tour &initial_tour, const InputData &input_data, const std::vector<int> &new1_to_old0);
