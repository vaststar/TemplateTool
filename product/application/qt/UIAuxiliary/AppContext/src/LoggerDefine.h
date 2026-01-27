#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* AppContextLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* AppContextLoggerTag = "AppContext";

#define AppContext_LOG_DEBUG(message)     UCF_LOG_DEBUG(AppContextLoggerTag, message, AppContextLoggerName)
#define AppContext_LOG_INFO(message)      UCF_LOG_INFO(AppContextLoggerTag, message, AppContextLoggerName)
#define AppContext_LOG_WARN(message)      UCF_LOG_WARN(AppContextLoggerTag, message, AppContextLoggerName)
#define AppContext_LOG_ERROR(message)     UCF_LOG_ERROR(AppContextLoggerTag, message, AppContextLoggerName)
#define AppContext_LOG_FATAL(message)     UCF_LOG_FATAL(AppContextLoggerTag, message, AppContextLoggerName)