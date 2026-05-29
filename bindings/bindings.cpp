#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "solver.h"
#include "problem_arguments.hpp"

namespace py = pybind11;

PYBIND11_MODULE(_tdtsp, m) {
    m.doc() = "VNS Fair VRP Solver";
    py::class_<InputData>(m, "InputData")
        .def(py::init<>())
        .def_readwrite("points_count", &InputData::points_count)
        .def_readwrite("min_load", &InputData::min_load)
        .def_readwrite("max_load", &InputData::max_load)
        .def_readwrite("max_time", &InputData::max_time)
        .def_readwrite("max_distance", &InputData::max_distance)
        .def_readwrite("distance_matrix", &InputData::distance_matrix)
        .def_readwrite("time_matrix", &InputData::time_matrix)
        .def_readwrite("point_scores", &InputData::point_scores)
        .def_readwrite("point_service_times", &InputData::point_service_times)
        .def("get_time_dependent_cost", &InputData::GetTimeDependentCost,
             py::arg("time"), py::arg("from"), py::arg("to"));

    py::class_<AgentSolution>(m, "AgentSolution")
        .def(py::init<>())
        .def_readwrite("route", &AgentSolution::route)
        .def_readwrite("solution_size", &AgentSolution::solution_size)
        .def_readwrite("total_time", &AgentSolution::total_time)
        .def_readwrite("total_distance", &AgentSolution::total_distance)
        .def_readwrite("total_value", &AgentSolution::total_value);

    py::class_<BenchmarkMetadata>(m, "BenchmarkMetadata")
        .def(py::init<>())
        .def_readwrite("st", &BenchmarkMetadata::ST)
        .def_readwrite("aon", &BenchmarkMetadata::AON)
        .def_readwrite("max_iter", &BenchmarkMetadata::max_iter)
        .def_readwrite("time_limit", &BenchmarkMetadata::time_limit)
        .def_readwrite("execution_time", &BenchmarkMetadata::execution_time);

    py::class_<Solution>(m, "Solution")
        .def(py::init<>())
        .def_readwrite("agents", &Solution::agents)
        .def_readwrite("meta", &Solution::meta);

    py::class_<Solver, std::shared_ptr<Solver>>(m, "Solver")
        .def("solve", &Solver::Solve, py::arg("instance"));

    py::class_<BaselineSolver, Solver, std::shared_ptr<BaselineSolver>>(m, "BaselineSolver")
        .def(py::init<double, int, int, int>(),
             py::arg("st"),
             py::arg("aon"),
             py::arg("max_iter"),
             py::arg("time_limit"))
        .def("solve", &BaselineSolver::Solve, py::arg("instance"));

    py::class_<AnnealingSolver, Solver, std::shared_ptr<AnnealingSolver>>(m, "AnnealingSolver")
        .def(py::init<double, int, int, int, double>(),
             py::arg("st"),
             py::arg("aon"),
             py::arg("max_iter"),
             py::arg("time_limit"),
             py::arg("alpha"))
        .def("solve", &AnnealingSolver::Solve, py::arg("instance"));

    py::class_<RebalancingSolver, Solver, std::shared_ptr<RebalancingSolver>>(m, "RebalancingSolver")
        .def(py::init<double, int, int, int, double>(),
             py::arg("st"),
             py::arg("aon"),
             py::arg("max_iter"),
             py::arg("time_limit"),
             py::arg("fairness"))
        .def("solve", &RebalancingSolver::Solve, py::arg("instance"));
}