#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* MainUILoggerName = ucf::utilities::kAppLoggerName;
static inline const char* MainUILoggerTag = "MainUI";

#define MAINUI_LOG_DEBUG(message)     UCF_LOG_DEBUG(MainUILoggerTag, message, MainUILoggerName)
#define MAINUI_LOG_INFO(message)      UCF_LOG_INFO(MainUILoggerTag, message, MainUILoggerName)
#define MAINUI_LOG_WARN(message)      UCF_LOG_WARN(MainUILoggerTag, message, MainUILoggerName)
#define MAINUI_LOG_ERROR(message)     UCF_LOG_ERROR(MainUILoggerTag, message, MainUILoggerName)
#define MAINUI_LOG_FATAL(message)     UCF_LOG_FATAL(MainUILoggerTag, message, MainUILoggerName)

// Logger lifecycle
#define LOG_WAIT_EXIT()     UCF_LOG_STOP()