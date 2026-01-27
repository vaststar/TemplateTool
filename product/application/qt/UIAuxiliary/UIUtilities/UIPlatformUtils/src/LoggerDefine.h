#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* UIPlatformUtilsLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* UIPlatformUtilsLoggerTag = "UIPlatformUtils";

#define UIPlatformUtils_LOG_DEBUG(message)     UCF_LOG_DEBUG(UIPlatformUtilsLoggerTag, message, UIPlatformUtilsLoggerName)
#define UIPlatformUtils_LOG_INFO(message)      UCF_LOG_INFO(UIPlatformUtilsLoggerTag, message, UIPlatformUtilsLoggerName)
#define UIPlatformUtils_LOG_WARN(message)      UCF_LOG_WARN(UIPlatformUtilsLoggerTag, message, UIPlatformUtilsLoggerName)
#define UIPlatformUtils_LOG_ERROR(message)     UCF_LOG_ERROR(UIPlatformUtilsLoggerTag, message, UIPlatformUtilsLoggerName)
#define UIPlatformUtils_LOG_FATAL(message)     UCF_LOG_FATAL(UIPlatformUtilsLoggerTag, message, UIPlatformUtilsLoggerName)