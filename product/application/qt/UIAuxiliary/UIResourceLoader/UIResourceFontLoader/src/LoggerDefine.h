#pragma once

#include <MasterLog/LogExport.h>

static inline const char * UIResourceFontLoaderLoggerName = "APP";
#define UIResourceFontLoader_LOG_DEBUG(message)     LOG_DEBUG("UIResourceFontLoader",message,UIResourceFontLoaderLoggerName)
#define UIResourceFontLoader_LOG_INFO(message)      LOG_INFO("UIResourceFontLoader",message,UIResourceFontLoaderLoggerName)
#define UIResourceFontLoader_LOG_WARN(message)      LOG_WARN("UIResourceFontLoader",message,UIResourceFontLoaderLoggerName)
#define UIResourceFontLoader_LOG_ERROR(message)     LOG_ERROR("UIResourceFontLoader",message,UIResourceFontLoaderLoggerName)
#define UIResourceFontLoader_LOG_FATAL(message)     LOG_FATAL("UIResourceFontLoader",message,UIResourceFontLoaderLoggerName)