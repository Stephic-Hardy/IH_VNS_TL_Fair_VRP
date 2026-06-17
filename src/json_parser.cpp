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

    inline void from_json(const json &j, AgentSolution &s) {
        j.at("route").get_to(s.route);
        j.at("total_time").get_to(s.total_time);
        j.at("total_distance").get_to(s.total_distance);
        j.at("total_value").get_to(s.total_value);
    }
}

namespace JsonParser {

    using Json = nlohmann::json;

    bool ParseInputDataFromJson(const std::string &json_path, InputData &arg) {
        std::ifstream json_file(json_path);
        if (!json_file) {
            std::cerr << "Can`t open input file with problem" << std::endl;
            return false;
        }

        Json j;
        json_file >> j;

        arg = j.get<InputData>();
        return true;
    }

    bool WriteSolutionToJsonFile(const std::string &json_path, const Solution &solution) {
        nlohmann::json j_root;
        nlohmann::json j_meta;
        
        const auto &meta = solution.meta;
        
        j_meta["ST"] = meta.st;
        j_meta["AON"] = meta.aon;
        j_meta["max_iter"] = meta.max_iter;
        j_meta["time_limit"] = meta.time_limit;
        j_meta["execution_time_sec"] = meta.execution_time;
        
        j_meta["global_vns_time"] = meta.stats.global_vns_time;
        j_meta["global_vns_iterations"] = meta.stats.global_vns_iterations;
        j_meta["advanced_vns_time"] = meta.stats.advanced_vns_time;
        j_meta["advanced_vns_iterations"] = meta.stats.advanced_vns_iterations;
        j_meta["local_vns_time"] = meta.stats.local_vns_time;
        j_meta["local_vns_iterations"] = meta.stats.local_vns_iterations;
        
        if (meta.stats.global_vns_time > 0) {
            j_meta["global_iterations_per_sec"] = static_cast<double>(meta.stats.global_vns_iterations) / meta.stats.global_vns_time;
        } else {
            j_meta["global_iterations_per_sec"] = 0.0;
        }
        
        if (meta.stats.advanced_vns_time > 0) {
            j_meta["advanced_iterations_per_sec"] = static_cast<double>(meta.stats.advanced_vns_iterations) / meta.stats.advanced_vns_time;
        } else {
            j_meta["advanced_iterations_per_sec"] = 0.0;
        }
        
        if (meta.stats.local_vns_time > 0) {
            j_meta["local_iterations_per_sec"] = static_cast<double>(meta.stats.local_vns_iterations) / meta.stats.local_vns_time;
        } else {
            j_meta["local_iterations_per_sec"] = 0.0;
        }
        
        j_root["metadata"] = j_meta;

        nlohmann::json j_array = nlohmann::json::array();
        for (const auto& s : solution.agents) {
            nlohmann::json j_obj;
            j_obj["route"] = s.route;
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