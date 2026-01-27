#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* UIViewLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* UIViewLoggerTag = "UIView";

#define UIVIEW_LOG_DEBUG(message)     UCF_LOG_DEBUG(UIViewLoggerTag, message, UIViewLoggerName)
#define UIVIEW_LOG_INFO(message)      UCF_LOG_INFO(UIViewLoggerTag, message, UIViewLoggerName)
#define UIVIEW_LOG_WARN(message)      UCF_LOG_WARN(UIViewLoggerTag, message, UIViewLoggerName)
#define UIVIEW_LOG_ERROR(message)     UCF_LOG_ERROR(UIViewLoggerTag, message, UIViewLoggerName)
#define UIVIEW_LOG_FATAL(message)     UCF_LOG_FATAL(UIViewLoggerTag, message, UIViewLoggerName)