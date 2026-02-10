#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* UICoreLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* UICoreLoggerTag = "UICore";

#define UICore_LOG_DEBUG(message)     UCF_LOG_DEBUG(UICoreLoggerTag, message, UICoreLoggerName)
#define UICore_LOG_INFO(message)      UCF_LOG_INFO(UICoreLoggerTag, message, UICoreLoggerName)
#define UICore_LOG_WARN(message)      UCF_LOG_WARN(UICoreLoggerTag, message, UICoreLoggerName)
#define UICore_LOG_ERROR(message)     UCF_LOG_ERROR(UICoreLoggerTag, message, UICoreLoggerName)
#define UICore_LOG_FATAL(message)     UCF_LOG_FATAL(UICoreLoggerTag, message, UICoreLoggerName)