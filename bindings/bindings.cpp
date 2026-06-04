#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "quill/Backend.h"

#include "solver.h"
#include "problem_arguments.hpp"

namespace py = pybind11;

PYBIND11_MODULE(tdtsp_python, m) {
    quill::Backend::start();
    
    m.doc() = "VNS Fair VRP Solver";
    py::class_<InputData>(m, "InputData")
        .def(py::init<size_t, size_t, size_t, size_t, size_t, 
            decltype(InputData::distance_matrix), 
            decltype(InputData::time_matrix), 
            decltype(InputData::point_scores), 
            decltype(InputData::point_service_times)>(),
            py::arg("points_count"),
            py::arg("min_load"),
            py::arg("max_load"),
            py::arg("max_time"),
            py::arg("max_distance"),
            py::arg("distance_matrix"),
            py::arg("time_matrix"),
            py::arg("point_scores"),
            py::arg("point_service_times"))
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
             py::arg("time"), py::arg("start"), py::arg("finish"));

    py::class_<AgentSolution>(m, "AgentSolution")
        .def(py::init<>())
        .def_readonly("route", &AgentSolution::route)
        .def_readonly("solution_size", &AgentSolution::solution_size)
        .def_readonly("total_time", &AgentSolution::total_time)
        .def_readonly("total_distance", &AgentSolution::total_distance)
        .def_readonly("total_value", &AgentSolution::total_value);

    py::class_<BenchmarkMetadata>(m, "BenchmarkMetadata")
        .def(py::init<>())
        .def_readonly("st", &BenchmarkMetadata::st)
        .def_readonly("aon", &BenchmarkMetadata::aon)
        .def_readonly("max_iter", &BenchmarkMetadata::max_iter)
        .def_readonly("time_limit", &BenchmarkMetadata::time_limit)
        .def_readonly("execution_time", &BenchmarkMetadata::execution_time)
        .def_readonly("fairness", &BenchmarkMetadata::fairness)
        .def_readonly("alpha", &BenchmarkMetadata::alpha);

    py::class_<Solution>(m, "Solution")
        .def(py::init<>())
        .def_readonly("agents", &Solution::agents)
        .def_readonly("meta", &Solution::meta);

    py::class_<Solver, std::shared_ptr<Solver>>(m, "Solver")
        .def("solve", &Solver::Solve, py::call_guard<py::gil_scoped_release>(), py::arg("instance"));

    py::class_<BaselineSolver, Solver, std::shared_ptr<BaselineSolver>>(m, "BaselineSolver")
        .def(py::init<double, int, int, int>(),
             py::arg("st"),
             py::arg("aon"),
             py::arg("max_iter"),
             py::arg("time_limit"));

    py::class_<AnnealingSolver, Solver, std::shared_ptr<AnnealingSolver>>(m, "AnnealingSolver")
        .def(py::init<double, int, int, int, double>(),
             py::arg("st"),
             py::arg("aon"),
             py::arg("max_iter"),
             py::arg("time_limit"),
             py::arg("alpha") = 0.5);

    py::class_<RebalancingSolver, Solver, std::shared_ptr<RebalancingSolver>>(m, "RebalancingSolver")
        .def(py::init<double, int, int, int, double>(),
             py::arg("st"),
             py::arg("aon"),
             py::arg("max_iter"),
             py::arg("time_limit"),
             py::arg("fairness"));
}