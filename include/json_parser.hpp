#pragma once

#include "problem_arguments.hpp"
#include <nlohmann/json.hpp>

namespace JsonParser {
bool ParseInputDataFromJson(const std::string& json_path, InputData& arg);

bool WriteSolutionToJsonFile(const std::string& json_path, const Solution& solutions);
}