#pragma once

#include "problem_arguments.hpp"

#include <quill/Logger.h>

quill::Logger *CreateOrGetLogger(std::string name, int verbose = 2);

void PrintVNSThroughput(quill::Logger *logger, const Solution& solution);

void PrintGiniDistance(quill::Logger *logger, const Solution& solution);
