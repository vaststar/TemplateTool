#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* FrameworkLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* TimerSchedulerLoggerTag = "TimerScheduler";

#define TS_LOG_DEBUG(message) UCF_LOG_DEBUG(TimerSchedulerLoggerTag, message, FrameworkLoggerName)
#define TS_LOG_INFO(message)  UCF_LOG_INFO(TimerSchedulerLoggerTag, message, FrameworkLoggerName)
#define TS_LOG_WARN(message)  UCF_LOG_WARN(TimerSchedulerLoggerTag, message, FrameworkLoggerName)
#define TS_LOG_ERROR(message) UCF_LOG_ERROR(TimerSchedulerLoggerTag, message, FrameworkLoggerName)
