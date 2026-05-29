#pragma once

#include "route_pack.h"

class Optimizer {
public:
    virtual ~Optimizer() = default;
    virtual RoutePack Optimize(RoutePack&, const InputData&) = 0;
};

class BaselineOptimizer : public Optimizer {
public:
    BaselineOptimizer(double st, int aon, int max_iter, int time_limit);
    ~BaselineOptimizer() override = default;
    RoutePack Optimize(RoutePack&, const InputData&) override;

private:
    const double st_;
    const int aon_;
    const int max_iter_;
    const int time_limit_;
};

class AnnealingOptimizer : public Optimizer {
public:
    AnnealingOptimizer(double st, int aon, int max_iter, int time_limit, double alpha);
    ~AnnealingOptimizer() override = default;
    RoutePack Optimize(RoutePack&, const InputData&) override;

private:
    const double st_;
    const int aon_;
    const int max_iter_;
    const int time_limit_;
    const double alpha_;
};

class RebalancingOptimizer : public Optimizer {
public:
    RebalancingOptimizer(double st, int aon, int max_iter, int time_limit, double fairness);
    ~RebalancingOptimizer() override = default;
    RoutePack Optimize(RoutePack&, const InputData&) override;

private:
    const double st_;
    const int aon_;
    const int max_iter_;
    const int time_limit_;
    const double fairness_;
};
