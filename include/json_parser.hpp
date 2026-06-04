#pragma once

#include "problem_arguments.hpp"
#include <nlohmann/json.hpp>

namespace JsonParser {
bool ParseInputDataFromJson(const std::string& json_path, InputData& arg);

bool ParseSolutionFromJson(const std::string& json_path, AgentSolution& solution);

bool WriteSolutionToJsonFile(const std::string& json_path, AgentSolution&& solution);

bool WriteMultiSolutionToJsonFile(const std::string& json_path,
                                  const std::vector<AgentSolution>& solutions);

bool WriteBenchmarkToJsonFile(const std::string& json_path, const Solution& solutions,
                              const BenchmarkMetadata& meta);
}