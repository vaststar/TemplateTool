#pragma once

#include <MasterLog/LogExport.h>

static inline const char * RunnerLoggerName = "APP";
#define RUNNER_LOG_DEBUG(message)     LOG_DEBUG("AppRunner",message,RunnerLoggerName)
#define RUNNER_LOG_INFO(message)      LOG_INFO("AppRunner",message,RunnerLoggerName)
#define RUNNER_LOG_WARN(message)      LOG_WARN("AppRunner",message,RunnerLoggerName)
#define RUNNER_LOG_ERROR(message)     LOG_ERROR("AppRunner",message,RunnerLoggerName)
#define RUNNER_LOG_FATAL(message)     LOG_FATAL("AppRunner",message,RunnerLoggerName)