#include "problem_arguments.hpp"
#include <unistd.h>
#include <iostream>



bool ParseProgramArguments(int argc, char *argv[], ProgramArguments &args) {
    int opt;
    while ((opt = getopt(argc, argv, "p:s:t:")) != -1) {
        switch (opt) {
            case 'p': {
                args.problemJsonPath = optarg;
                break;
            }
            case 's': {
                args.solutionJsonPath = optarg;
                break;
            }
            case 't': {
                args.time = std::stoull(optarg);
                break;
            }
            case 'h':
                [[fallthrough]];
            default:
                std::cerr << "Error in program`s arguments.\n"
                          << "Correct format:\n"
                          << "-p <path to problem JSON>\n"
                          << "-s <path to solution JSON>\n"
                          << "-t <time to work in second stage>\n";
                return false;
        }
    }
#ifdef DEBUG
    std::cout << "Problem path: " << args.problemJsonPath <<
              " solution path: " << args.solutionJsonPath <<
              " time: " << args.time << std::endl;
#endif

    return true;
}

int64_t InputData::GetTimeDependentCost(uint64_t time, uint64_t from, uint64_t to) const {
    if (time >= kTimeDuration * (time_matrix.size() - 1)) {
        return time_matrix[time_matrix.size() - 1][from][to];
    }

    const auto time_matrix_idx = time / kTimeDuration;
    const double alpha = static_cast<double>(time - kTimeDuration * time_matrix_idx) / kTimeDuration;

    return static_cast<int64_t>(alpha * time_matrix[time_matrix_idx][from][to] +
                                 (1 - alpha) * time_matrix[time_matrix_idx + 1][from][to]);
}

std::ostream &operator<<(std::ostream &os, const InputData &data) {
    os << "points_count: " << data.points_count << "\n";
    os << "min_load: " << data.min_load << "\n";
    os << "max_load: " << data.max_load << "\n";
    os << "max_time: " << data.max_time << "\n";
    os << "max_distance: " << data.max_distance << "\n";

    os << "distance_matrix:\n";
    for (const auto &row: data.distance_matrix) {
        for (auto d: row) {
            os << d << " ";
        }
        os << "\n";
    }

    os << "time_matrix:\n";
    for (size_t t = 0; t < data.time_matrix.size(); ++t) {
        os << "Time step " << t << ":\n";
        for (const auto &row: data.time_matrix[t]) {
            for (auto val: row) {
                os << val << " ";
            }
            os << "\n";
        }
    }

    os << "point_scores: ";
    for (auto val: data.point_scores) {
        os << val << " ";
    }
    os << "\n";

    os << "point_service_times: ";
    for (auto val: data.point_service_times) {
        os << val << " ";
    }
    os << "\n";

    return os;
}

std::ostream &operator<<(std::ostream &os, const AgentSolution &solution) {
    os << "solution_size: " << solution.solution_size << "\n";

    os << "route: ";
    for (auto idx: solution.route) {
        os << idx << " ";
    }
    os << "\n";

    os << "total_time: " << solution.total_time << "\n";
    os << "total_distance: " << solution.total_distance << "\n";
    os << "total_value: " << solution.total_value << "\n";

    return os;
}