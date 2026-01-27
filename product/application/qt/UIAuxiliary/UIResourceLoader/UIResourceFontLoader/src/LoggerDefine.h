#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* UIResourceFontLoaderLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* UIResourceFontLoaderLoggerTag = "UIResourceFontLoader";

#define UIResourceFontLoader_LOG_DEBUG(message)     UCF_LOG_DEBUG(UIResourceFontLoaderLoggerTag, message, UIResourceFontLoaderLoggerName)
#define UIResourceFontLoader_LOG_INFO(message)      UCF_LOG_INFO(UIResourceFontLoaderLoggerTag, message, UIResourceFontLoaderLoggerName)
#define UIResourceFontLoader_LOG_WARN(message)      UCF_LOG_WARN(UIResourceFontLoaderLoggerTag, message, UIResourceFontLoaderLoggerName)
#define UIResourceFontLoader_LOG_ERROR(message)     UCF_LOG_ERROR(UIResourceFontLoaderLoggerTag, message, UIResourceFontLoaderLoggerName)
#define UIResourceFontLoader_LOG_FATAL(message)     UCF_LOG_FATAL(UIResourceFontLoaderLoggerTag, message, UIResourceFontLoaderLoggerName)