#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* UIComponentBaseLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* UIComponentBaseLoggerTag = "UIComponentBase";

#define UICOMBASE_LOG_DEBUG(message)     UCF_LOG_DEBUG(UIComponentBaseLoggerTag, message, UIComponentBaseLoggerName)
#define UICOMBASE_LOG_INFO(message)      UCF_LOG_INFO(UIComponentBaseLoggerTag, message, UIComponentBaseLoggerName)
#define UICOMBASE_LOG_WARN(message)      UCF_LOG_WARN(UIComponentBaseLoggerTag, message, UIComponentBaseLoggerName)
#define UICOMBASE_LOG_ERROR(message)     UCF_LOG_ERROR(UIComponentBaseLoggerTag, message, UIComponentBaseLoggerName)
#define UICOMBASE_LOG_FATAL(message)     UCF_LOG_FATAL(UIComponentBaseLoggerTag, message, UIComponentBaseLoggerName)