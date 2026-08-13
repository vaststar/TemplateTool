#pragma once

#include <ucf/utilities/LoggerUtils/LoggerExport.h>

static inline const char* UIViewSupportLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* UIViewSupportLoggerTag = "UIViewSupport";

#define UIVIEW_SUPPORT_LOG_DEBUG(message) UCF_LOG_DEBUG(UIViewSupportLoggerTag, message, UIViewSupportLoggerName)
#define UIVIEW_SUPPORT_LOG_INFO(message)  UCF_LOG_INFO(UIViewSupportLoggerTag, message, UIViewSupportLoggerName)
#define UIVIEW_SUPPORT_LOG_WARN(message)  UCF_LOG_WARN(UIViewSupportLoggerTag, message, UIViewSupportLoggerName)
#define UIVIEW_SUPPORT_LOG_ERROR(message) UCF_LOG_ERROR(UIViewSupportLoggerTag, message, UIViewSupportLoggerName)
#define UIVIEW_SUPPORT_LOG_FATAL(message) UCF_LOG_FATAL(UIViewSupportLoggerTag, message, UIViewSupportLoggerName)
