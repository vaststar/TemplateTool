#pragma once

#include <MasterLog/LogExport.h>

static inline const char * UIResourceColorLoaderLoggerName = "APP";
#define UIResourceColorLoader_LOG_DEBUG(message)     LOG_DEBUG("UIResourceColorLoader",message,UIResourceColorLoaderLoggerName)
#define UIResourceColorLoader_LOG_INFO(message)      LOG_INFO("UIResourceColorLoader",message,UIResourceColorLoaderLoggerName)
#define UIResourceColorLoader_LOG_WARN(message)      LOG_WARN("UIResourceColorLoader",message,UIResourceColorLoaderLoggerName)
#define UIResourceColorLoader_LOG_ERROR(message)     LOG_ERROR("UIResourceColorLoader",message,UIResourceColorLoaderLoggerName)
#define UIResourceColorLoader_LOG_FATAL(message)     LOG_FATAL("UIResourceColorLoader",message,UIResourceColorLoaderLoggerName)