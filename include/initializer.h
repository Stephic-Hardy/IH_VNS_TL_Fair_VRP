#pragma once

#include "route_pack.h"
#include "problem_arguments.hpp"

class Initializer {
public:
    virtual ~Initializer() = default;
    virtual RoutePack BuildInitialRoutes(const InputData& instance) = 0;
};

class InsertionHeuristicInitializer : public Initializer {
public:
    InsertionHeuristicInitializer() = default;
    ~InsertionHeuristicInitializer() override = default;
    RoutePack BuildInitialRoutes(const InputData& instance) override;
};
