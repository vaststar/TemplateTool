#pragma once

#include <ucf/utilities/LoggerUtils/LoggerExport.h>

static inline const char* UIViewCoreLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* UIViewCoreLoggerTag = "UIViewCore";

#define UIViewCore_LOG_DEBUG(message) UCF_LOG_DEBUG(UIViewCoreLoggerTag, message, UIViewCoreLoggerName)
#define UIViewCore_LOG_INFO(message)  UCF_LOG_INFO(UIViewCoreLoggerTag, message, UIViewCoreLoggerName)
#define UIViewCore_LOG_WARN(message)  UCF_LOG_WARN(UIViewCoreLoggerTag, message, UIViewCoreLoggerName)
#define UIViewCore_LOG_ERROR(message) UCF_LOG_ERROR(UIViewCoreLoggerTag, message, UIViewCoreLoggerName)
#define UIViewCore_LOG_FATAL(message) UCF_LOG_FATAL(UIViewCoreLoggerTag, message, UIViewCoreLoggerName)
