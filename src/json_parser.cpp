#include <fstream>
#include <iostream>
#include "problem_arguments.hpp"
#include "json_parser.hpp"

namespace nlohmann {
    inline void from_json(const json &j, InputData &t) {
        j.at("points_count").get_to(t.points_count);
        j.at("min_load").get_to(t.min_load);
        j.at("max_load").get_to(t.max_load);
        j.at("max_time").get_to(t.max_time);
        j.at("max_distance").get_to(t.max_distance);
        j.at("distance_matrix").get_to(t.distance_matrix);
        j.at("time_matrix").get_to(t.time_matrix);
        j.at("point_scores").get_to(t.point_scores);
        j.at("point_service_times").get_to(t.point_service_times);
    }

    inline void to_json(json &j, const AgentSolution &s) {
        j = json{
                {"route",          s.route},
                {"solution_size",  s.solution_size},
                {"total_time",     s.total_time},
                {"total_distance", s.total_distance},
                {"total_value",    s.total_value}
        };
    }

    inline void from_json(const json &j, AgentSolution &s) {
        j.at("route").get_to(s.route);
        j.at("solution_size").get_to(s.solution_size);
        j.at("total_time").get_to(s.total_time);
        j.at("total_distance").get_to(s.total_distance);
        j.at("total_value").get_to(s.total_value);
    }
}

namespace JsonParser {

    using json = nlohmann::json;

    bool ParseInputDataFromJson(const std::string &json_path, InputData &arg) {
        std::ifstream jsonFile(json_path);
        if (!jsonFile) {
            std::cerr << "Can`t open input file with problem" << std::endl;
            return false;
        }

        json j;
        jsonFile >> j;

        arg = j.get<InputData>();
        return true;
    }

    bool ParseSolutionFromJson(const std::string &json_path, AgentSolution &solution) {
        std::ifstream json_file(json_path);
        if (!json_file) {
            std::cerr << "Can`t open input file with solution" << std::endl;
            return false;
        }

        json j;
        json_file >> j;

        solution = j.get<AgentSolution>();
        return true;
    }

    bool WriteSolutionToJsonFile(const std::string &json_path, AgentSolution &&solution) {
        nlohmann::json j = std::move(solution);

        std::ofstream file(json_path);
        if (!file) {
            std::cerr << "Can`t open output file to write solution" << std::endl;
            return false;
        }

        file << j.dump(4);
        return true;
    };

    bool WriteMultiSolutionToJsonFile(const std::string &json_path, const std::vector<AgentSolution> &solutions) {
        // Создаем пустой JSON массив
        nlohmann::json j_array = nlohmann::json::array();

        for (const auto& s : solutions) {
            nlohmann::json j_obj;
            j_obj["route"] = s.route;
            j_obj["solution_size"] = s.solution_size;
            j_obj["total_time"] = s.total_time;
            j_obj["total_distance"] = s.total_distance;
            j_obj["total_value"] = s.total_value;
            
            j_array.push_back(j_obj);
        }

        std::ofstream file(json_path);
        if (!file.is_open()) {
            std::cerr << "Could not open file for writing: " << json_path << std::endl;
            return false;
        }

        // Записываем массив напрямую, как в твоем примере
        file << j_array.dump(4);
        return true;
    }

    bool WriteBenchmarkToJsonFile(const std::string &json_path, const Solution &solution, const BenchmarkMetadata& meta) {
        nlohmann::json j_root;

        nlohmann::json j_meta;
        j_meta["ST"] = meta.st;
        j_meta["AON"] = meta.aon;
        j_meta["max_iter"] = meta.max_iter;
        j_meta["time_limit"] = meta.time_limit;
        j_meta["execution_time_sec"] = meta.execution_time;
        j_root["metadata"] = j_meta;

        nlohmann::json j_array = nlohmann::json::array();
        for (const auto& s : solution.agents) {
            nlohmann::json j_obj;
            j_obj["route"] = s.route;
            j_obj["solution_size"] = s.solution_size;
            j_obj["total_time"] = s.total_time;
            j_obj["total_distance"] = s.total_distance;
            j_obj["total_value"] = s.total_value;
            j_array.push_back(j_obj);
        }
        j_root["solutions"] = j_array;

        std::ofstream file(json_path);
        if (!file.is_open()) {
            return false;
        }

        file << j_root.dump(4);
        return true;
    }
}