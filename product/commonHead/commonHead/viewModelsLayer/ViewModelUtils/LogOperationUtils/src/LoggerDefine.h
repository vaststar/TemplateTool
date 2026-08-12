#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* LogOperationUtilsLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* LogOperationUtilsLoggerTag = "LogOperationUtils";

#define LOG_OPERATION_UTILS_LOG_DEBUG(message) UCF_LOG_DEBUG(LogOperationUtilsLoggerTag, message, LogOperationUtilsLoggerName)
#define LOG_OPERATION_UTILS_LOG_INFO(message) UCF_LOG_INFO(LogOperationUtilsLoggerTag, message, LogOperationUtilsLoggerName)
#define LOG_OPERATION_UTILS_LOG_WARN(message) UCF_LOG_WARN(LogOperationUtilsLoggerTag, message, LogOperationUtilsLoggerName)
#define LOG_OPERATION_UTILS_LOG_ERROR(message) UCF_LOG_ERROR(LogOperationUtilsLoggerTag, message, LogOperationUtilsLoggerName)
#define LOG_OPERATION_UTILS_LOG_FATAL(message) UCF_LOG_FATAL(LogOperationUtilsLoggerTag, message, LogOperationUtilsLoggerName)
