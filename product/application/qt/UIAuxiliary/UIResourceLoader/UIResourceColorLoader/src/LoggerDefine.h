#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* UIResourceColorLoaderLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* UIResourceColorLoaderLoggerTag = "UIResourceColorLoader";

#define UIResourceColorLoader_LOG_DEBUG(message)     UCF_LOG_DEBUG(UIResourceColorLoaderLoggerTag, message, UIResourceColorLoaderLoggerName)
#define UIResourceColorLoader_LOG_INFO(message)      UCF_LOG_INFO(UIResourceColorLoaderLoggerTag, message, UIResourceColorLoaderLoggerName)
#define UIResourceColorLoader_LOG_WARN(message)      UCF_LOG_WARN(UIResourceColorLoaderLoggerTag, message, UIResourceColorLoaderLoggerName)
#define UIResourceColorLoader_LOG_ERROR(message)     UCF_LOG_ERROR(UIResourceColorLoaderLoggerTag, message, UIResourceColorLoaderLoggerName)
#define UIResourceColorLoader_LOG_FATAL(message)     UCF_LOG_FATAL(UIResourceColorLoaderLoggerTag, message, UIResourceColorLoaderLoggerName)