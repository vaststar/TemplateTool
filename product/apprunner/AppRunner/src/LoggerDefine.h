#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* RunnerLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* RunnerLoggerTag = "AppRunner";

#define RUNNER_LOG_DEBUG(message)     UCF_LOG_DEBUG(RunnerLoggerTag, message, RunnerLoggerName)
#define RUNNER_LOG_INFO(message)      UCF_LOG_INFO(RunnerLoggerTag, message, RunnerLoggerName)
#define RUNNER_LOG_WARN(message)      UCF_LOG_WARN(RunnerLoggerTag, message, RunnerLoggerName)
#define RUNNER_LOG_ERROR(message)     UCF_LOG_ERROR(RunnerLoggerTag, message, RunnerLoggerName)
#define RUNNER_LOG_FATAL(message)     UCF_LOG_FATAL(RunnerLoggerTag, message, RunnerLoggerName)