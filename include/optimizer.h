#pragma once

#include "route_pack.h"

#include <quill/Logger.h>

class Optimizer {
public:
    virtual ~Optimizer() = default;
    virtual void EnrichMeta(BenchmarkMetadata& meta) = 0;
    virtual RoutePack Optimize(RoutePack&, const InputData&) = 0;
    void SetLogger(quill::Logger*);

protected:
    quill::Logger* logger_ = nullptr;
    ExecutionStats stats_;
};

class BaselineOptimizer : public Optimizer {
public:
    BaselineOptimizer(double st, int aon, int max_iter, int time_limit);
    ~BaselineOptimizer() override = default;
    void EnrichMeta(BenchmarkMetadata& meta) override;
    RoutePack Optimize(RoutePack&, const InputData&) override;

private:
    const double st_;
    const int aon_;
    const int max_iter_;
    const double time_limit_;
};

class AnnealingOptimizer : public Optimizer {
public:
    AnnealingOptimizer(double st, int aon, int max_iter, int time_limit, double alpha);
    ~AnnealingOptimizer() override = default;
    void EnrichMeta(BenchmarkMetadata& meta) override;
    RoutePack Optimize(RoutePack&, const InputData&) override;

private:
    const double st_;
    const int aon_;
    const int max_iter_;
    const double time_limit_;
    const double alpha_;
};

class RebalancingOptimizer : public Optimizer {
public:
    RebalancingOptimizer(double st, int aon, int max_iter, int time_limit, double fairness);
    ~RebalancingOptimizer() override = default;
    void EnrichMeta(BenchmarkMetadata& meta) override;
    RoutePack Optimize(RoutePack&, const InputData&) override;

private:
    const double st_;
    const int aon_;
    const int max_iter_;
    const double time_limit_;
    const double fairness_;
};
