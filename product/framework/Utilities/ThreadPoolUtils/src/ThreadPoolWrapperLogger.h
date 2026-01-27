#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* ThreadPoolUtilsLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* ThreadPoolUtilsLoggerTag = "ThreadPoolUtils";

#define TPWRAPPER_LOG_DEBUG(message)  UCF_LOG_DEBUG(ThreadPoolUtilsLoggerTag, message, ThreadPoolUtilsLoggerName)
#define TPWRAPPER_LOG_INFO(message)   UCF_LOG_INFO(ThreadPoolUtilsLoggerTag, message, ThreadPoolUtilsLoggerName)
#define TPWRAPPER_LOG_WARN(message)   UCF_LOG_WARN(ThreadPoolUtilsLoggerTag, message, ThreadPoolUtilsLoggerName)
#define TPWRAPPER_LOG_ERROR(message)  UCF_LOG_ERROR(ThreadPoolUtilsLoggerTag, message, ThreadPoolUtilsLoggerName)
#define TPWRAPPER_LOG_FATAL(message)  UCF_LOG_FATAL(ThreadPoolUtilsLoggerTag, message, ThreadPoolUtilsLoggerName)
