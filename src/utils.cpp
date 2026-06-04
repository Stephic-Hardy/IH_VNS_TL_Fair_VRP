#include "utils.h"
#include "problem_arguments.hpp"
#include "quill/Logger.h"

#include <algorithm>
#include <numeric>

#include <quill/Backend.h>
#include <quill/Frontend.h>
#include <quill/LogMacros.h>
#include <quill/sinks/ConsoleSink.h>

quill::Logger *CreateOrGetLogger(std::string name, int verbose) {
    auto console_sink = quill::Frontend::create_or_get_sink<quill::ConsoleSink>("console_sink");

    quill::Logger *logger;
    if (verbose != 3) {
        quill::PatternFormatterOptions format_options;
        format_options.format_pattern =
            "[%(log_level)] %(message)";
        logger = quill::Frontend::create_or_get_logger(name, std::move(console_sink), format_options);
    } else {
        logger = quill::Frontend::create_or_get_logger(name, std::move(console_sink));
    }
    switch (verbose) {
        case 0:
            logger->set_log_level(quill::LogLevel::Error);
            break;
        case 1:
            logger->set_log_level(quill::LogLevel::Warning);
            break;
        case 2:
            logger->set_log_level(quill::LogLevel::Info);
            break;
        case 3:
            logger->set_log_level(quill::LogLevel::Debug);
            break;
        default:
            logger->set_log_level(quill::LogLevel::Info);
            LOG_WARNING(logger, "Invalid verbose level was specified: {}, defaulted to 2", verbose);
    }
    return logger;
}

void PrintGiniDistance(quill::Logger *logger, const Solution &solution) {
    if (solution.agents.empty()) {
        LOG_INFO(logger, "Нет данных для анализа.");
        return;
    }

    size_t n = solution.agents.size();
    std::vector<double> distances;
    for (const auto &sol : solution.agents) {
        distances.push_back(static_cast<double>(sol.total_distance));
    }

    std::sort(distances.begin(), distances.end());

    double sum = std::accumulate(distances.begin(), distances.end(), 0.0);

    if (sum == 0) {
        LOG_INFO(logger, "Нулевая дистанция, расчет невозможен.");
        return;
    }

    double weighted_sum = 0;
    for (size_t i = 0; i < n; ++i) {
        weighted_sum += (i + 1) * distances[i];
    }

    double gini = (2.0 * weighted_sum) / (n * sum) - (static_cast<double>(n) + 1.0) / n;

    double min_dist = std::ranges::min(distances);
    double max_dist = std::ranges::max(distances);

    LOG_INFO(logger, "--- FINAL STATISTICS ---");
    LOG_INFO(logger, "Средняя дистанция:   {:.2f}", sum / n);
    LOG_INFO(logger, "Коэффициент Джини:   {:.4f}", gini);
    LOG_INFO(logger, "(Max - Min):         {:.4f}", max_dist - min_dist);
    LOG_INFO(logger, "(Max - Min) / Min:   {:.4f}", (max_dist - min_dist) / min_dist);
}
