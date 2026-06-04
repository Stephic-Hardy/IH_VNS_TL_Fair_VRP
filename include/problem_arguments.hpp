#pragma once

#include <cstdint>
#include <vector>
#include <string>

struct ProgramArguments {
    std::string problemJsonPath;
    std::string solutionJsonPath;
    uint64_t time;
};

struct InputData {
    /// количество точек в задаче, включая склад.
    uint64_t points_count{};
    /// минимальная загрузка исполнителя
    uint64_t min_load{};
    /// максимальная загрузка исполнителя
    uint64_t max_load{};
    /// максимальное время в построенной миссии
    uint64_t max_time{};
    /// максимальное расстояние в построенной миссии
    uint64_t max_distance{};
    /// матрица расстояний между точками. Размерность матрицы
    std::vector<std::vector<int64_t>> distance_matrix;
    /// Матрица времени перемещения между точками.
    /// Размерность матрицы @time_steps x @points_count x @points_count,
    /// склад/стартовая точка исполнителя в матрице имеет индекс 0.
    std::vector<std::vector<std::vector<int64_t>>> time_matrix;
    /// массив "важностей" всех точек, кроме склада.
    /// Размерность массива @points_count - 1.
    std::vector<int64_t> point_scores;
    /// массив времён на обслуживание каждой точки, кроме склада.
    /// Размерность массива @points_count - 1.
    std::vector<int64_t> point_service_times;

    /// 30 минут в секундах для TD цены перехода
    static constexpr uint64_t kTimeDuration = 30 * 60;

    int64_t GetTimeDependentCost(uint64_t time, uint64_t from, uint64_t to) const;
};

std::ostream& operator<<(std::ostream& os, const InputData& data);

struct AgentSolution {
    /// последовательость индексов точек, составляющих найденный маршрут.
    std::vector<uint64_t> route;
    /// ETA на прохождение маршрута (по @time_matrix и @point_service_times).
    uint64_t total_time{};
    ///  суммарное расстояние в маршруте (по @distance_matrix).
    uint64_t total_distance{};
    /// суммарный скор найденного маршрута (по @point_scores и @time_matrix).
    uint64_t total_value{};
};

struct BenchmarkMetadata {
    double st;
    int aon;
    int max_iter;
    double time_limit;
    double execution_time;

    // Annealing sepcific
    double alpha = 0.5;
    
    // Rebalancing sepcific
    double fairness = 0.5;
};

struct Solution {
    std::vector<AgentSolution> agents;
    BenchmarkMetadata meta;
};

std::ostream& operator<<(std::ostream& os, const AgentSolution& solution);