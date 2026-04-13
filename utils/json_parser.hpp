#pragma once

#include "problem_arguments.hpp"
#include <nlohmann/json.hpp>

namespace JsonParser {
    bool ParseInputDataFromJson(const std::string &jsonPath, InputData &arg);

    bool ParseSolutionFromJson(const std::string &jsonPath, Solution& solution);

    bool WriteSolutionToJsonFile(const std::string &jsonPath, Solution &&solution);

    bool WriteMultiSolutionToJsonFile(const std::string &jsonPath, const std::vector<Solution> &solutions);
}