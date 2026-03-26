#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline constexpr auto PERF_LOGGER_NAME = ucf::utilities::kAppLoggerName;
static inline constexpr auto PERF_LOGGER_TAG = "PerformanceService";

#define PERFORMANCE_LOG_DEBUG(message)  UCF_LOG_DEBUG(PERF_LOGGER_TAG, message, PERF_LOGGER_NAME)
#define PERFORMANCE_LOG_INFO(message)   UCF_LOG_INFO(PERF_LOGGER_TAG, message, PERF_LOGGER_NAME)
#define PERFORMANCE_LOG_WARN(message)   UCF_LOG_WARN(PERF_LOGGER_TAG, message, PERF_LOGGER_NAME)
#define PERFORMANCE_LOG_ERROR(message)  UCF_LOG_ERROR(PERF_LOGGER_TAG, message, PERF_LOGGER_NAME)
