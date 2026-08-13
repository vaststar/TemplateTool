#pragma once

#include <ucf/utilities/LoggerUtils/LoggerExport.h>

static inline const char* AppUILoggerName = ucf::utilities::kAppLoggerName;
static inline const char* AppUILoggerTag = "AppUI";

#define APPUI_LOG_DEBUG(message) UCF_LOG_DEBUG(AppUILoggerTag, message, AppUILoggerName)
#define APPUI_LOG_INFO(message)  UCF_LOG_INFO(AppUILoggerTag, message, AppUILoggerName)
#define APPUI_LOG_WARN(message)  UCF_LOG_WARN(AppUILoggerTag, message, AppUILoggerName)
#define APPUI_LOG_ERROR(message) UCF_LOG_ERROR(AppUILoggerTag, message, AppUILoggerName)
#define APPUI_LOG_FATAL(message) UCF_LOG_FATAL(AppUILoggerTag, message, AppUILoggerName)
