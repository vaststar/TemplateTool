#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* UIManagerLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* UIManagerLoggerTag = "UIManager";

#define UIManager_LOG_DEBUG(message)     UCF_LOG_DEBUG(UIManagerLoggerTag, message, UIManagerLoggerName)
#define UIManager_LOG_INFO(message)      UCF_LOG_INFO(UIManagerLoggerTag, message, UIManagerLoggerName)
#define UIManager_LOG_WARN(message)      UCF_LOG_WARN(UIManagerLoggerTag, message, UIManagerLoggerName)
#define UIManager_LOG_ERROR(message)     UCF_LOG_ERROR(UIManagerLoggerTag, message, UIManagerLoggerName)
#define UIManager_LOG_FATAL(message)     UCF_LOG_FATAL(UIManagerLoggerTag, message, UIManagerLoggerName)
