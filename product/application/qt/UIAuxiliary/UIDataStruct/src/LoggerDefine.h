#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* UIDataLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* UIDataLoggerTag = "UIData";

#define UIData_LOG_DEBUG(message)     UCF_LOG_DEBUG(UIDataLoggerTag, message, UIDataLoggerName)
#define UIData_LOG_INFO(message)      UCF_LOG_INFO(UIDataLoggerTag, message, UIDataLoggerName)
#define UIData_LOG_WARN(message)      UCF_LOG_WARN(UIDataLoggerTag, message, UIDataLoggerName)
#define UIData_LOG_ERROR(message)     UCF_LOG_ERROR(UIDataLoggerTag, message, UIDataLoggerName)
#define UIData_LOG_FATAL(message)     UCF_LOG_FATAL(UIDataLoggerTag, message, UIDataLoggerName)