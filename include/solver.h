#pragma once

#include "initializer.h"
#include "optimizer.h"
#include "problem_arguments.hpp"
#include <memory>

class Solver {
public:
    Solver(std::unique_ptr<Initializer> initializer, std::unique_ptr<Optimizer> optimizer);
    virtual ~Solver() = default;
    Solution Solve(const InputData& input_data);

protected:
    std::unique_ptr<Initializer> initializer_;
    std::unique_ptr<Optimizer> optimizer_;
};

class BaselineSolver final : public Solver {
public:
    BaselineSolver(double st, int aon, int max_iter, int time_limit);
    ~BaselineSolver() override = default;
};

class AnnealingSolver final : public Solver {
public:
    AnnealingSolver(double st, int aon, int max_iter, int time_limit, double alpha);
    ~AnnealingSolver() override = default;
};

class RebalancingSolver final : public Solver {
public:
    RebalancingSolver(double st, int aon, int max_iter, int time_limit, double fairness);
    ~RebalancingSolver() override = default;
};
