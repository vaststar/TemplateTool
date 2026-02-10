#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* UIResourceStringLoaderLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* UIResourceStringLoaderLoggerTag = "UIResourceStringLoader";

#define UIResourceStringLoader_LOG_DEBUG(message)     UCF_LOG_DEBUG(UIResourceStringLoaderLoggerTag, message, UIResourceStringLoaderLoggerName)
#define UIResourceStringLoader_LOG_INFO(message)      UCF_LOG_INFO(UIResourceStringLoaderLoggerTag, message, UIResourceStringLoaderLoggerName)
#define UIResourceStringLoader_LOG_WARN(message)      UCF_LOG_WARN(UIResourceStringLoaderLoggerTag, message, UIResourceStringLoaderLoggerName)
#define UIResourceStringLoader_LOG_ERROR(message)     UCF_LOG_ERROR(UIResourceStringLoaderLoggerTag, message, UIResourceStringLoaderLoggerName)
#define UIResourceStringLoader_LOG_FATAL(message)     UCF_LOG_FATAL(UIResourceStringLoaderLoggerTag, message, UIResourceStringLoaderLoggerName)