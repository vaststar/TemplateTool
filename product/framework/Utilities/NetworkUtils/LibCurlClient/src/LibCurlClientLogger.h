#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* FrameworkLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* FrameworkLoggerTag = "LibCurlClient";

#define LIBCURL_LOG_DEBUG(message)     UCF_LOG_DEBUG(FrameworkLoggerTag, message, FrameworkLoggerName)
#define LIBCURL_LOG_INFO(message)      UCF_LOG_INFO(FrameworkLoggerTag, message, FrameworkLoggerName)
#define LIBCURL_LOG_WARN(message)      UCF_LOG_WARN(FrameworkLoggerTag, message, FrameworkLoggerName)
#define LIBCURL_LOG_ERROR(message)     UCF_LOG_ERROR(FrameworkLoggerTag, message, FrameworkLoggerName)
#define LIBCURL_LOG_FATAL(message)     UCF_LOG_FATAL(FrameworkLoggerTag, message, FrameworkLoggerName)