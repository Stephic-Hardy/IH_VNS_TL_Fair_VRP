#include <fstream>
#include <iostream>
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

    inline void to_json(json &j, const Solution &s) {
        j = json{
                {"route",          s.route},
                {"solution_size",  s.solution_size},
                {"total_time",     s.total_time},
                {"total_distance", s.total_distance},
                {"total_value",    s.total_value}
        };
    }

    inline void from_json(const json &j, Solution &s) {
        j.at("route").get_to(s.route);
        j.at("solution_size").get_to(s.solution_size);
        j.at("total_time").get_to(s.total_time);
        j.at("total_distance").get_to(s.total_distance);
        j.at("total_value").get_to(s.total_value);
    }
}

namespace JsonParser {

    using json = nlohmann::json;

    bool ParseInputDataFromJson(const std::string &jsonPath, InputData &arg) {
        std::ifstream jsonFile(jsonPath);
        if (!jsonFile) {
            std::cerr << "Can`t open input file with problem" << std::endl;
            return false;
        }

        json j;
        jsonFile >> j;

        arg = j.get<InputData>();
        return true;
    }

    bool ParseSolutionFromJson(const std::string &jsonPath, Solution &solution) {
        std::ifstream jsonFile(jsonPath);
        if (!jsonFile) {
            std::cerr << "Can`t open input file with solution" << std::endl;
            return false;
        }

        json j;
        jsonFile >> j;

        solution = j.get<Solution>();
        return true;
    }

    bool WriteSolutionToJsonFile(const std::string &jsonPath, Solution &&solution) {
        nlohmann::json j = std::move(solution);

        std::ofstream file(jsonPath);
        if (!file) {
            std::cerr << "Can`t open output file to write solution" << std::endl;
            return false;
        }

        file << j.dump(4);
        return true;
    };

    bool WriteMultiSolutionToJsonFile(const std::string &jsonPath, const std::vector<Solution> &solutions) {
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

        std::ofstream file(jsonPath);
        if (!file.is_open()) {
            std::cerr << "Could not open file for writing: " << jsonPath << std::endl;
            return false;
        }

        // Записываем массив напрямую, как в твоем примере
        file << j_array.dump(4);
        return true;
    }
}