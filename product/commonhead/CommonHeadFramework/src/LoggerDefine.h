#pragma once

#include <ucf/utilities/LoggerUtils/LoggerExport.h>

static inline const char* CommonHeadFrameworkLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* CommonHeadFrameworkLoggerTag = "CommonHeadFramework";

#define COMMON_HEAD_FRAMEWORK_LOG_DEBUG(message) UCF_LOG_DEBUG(CommonHeadFrameworkLoggerTag, message, CommonHeadFrameworkLoggerName)
#define COMMON_HEAD_FRAMEWORK_LOG_INFO(message) UCF_LOG_INFO(CommonHeadFrameworkLoggerTag, message, CommonHeadFrameworkLoggerName)
#define COMMON_HEAD_FRAMEWORK_LOG_WARN(message) UCF_LOG_WARN(CommonHeadFrameworkLoggerTag, message, CommonHeadFrameworkLoggerName)
#define COMMON_HEAD_FRAMEWORK_LOG_ERROR(message) UCF_LOG_ERROR(CommonHeadFrameworkLoggerTag, message, CommonHeadFrameworkLoggerName)
#define COMMON_HEAD_FRAMEWORK_LOG_FATAL(message) UCF_LOG_FATAL(CommonHeadFrameworkLoggerTag, message, CommonHeadFrameworkLoggerName)
