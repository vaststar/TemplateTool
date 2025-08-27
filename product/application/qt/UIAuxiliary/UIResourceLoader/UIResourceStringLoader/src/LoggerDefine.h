#pragma once

#include <MasterLog/LogExport.h>

static inline const char * UIResourceStringLoaderLoggerName = "APP";
#define UIResourceStringLoader_LOG_DEBUG(message)     LOG_DEBUG("UIResourceStringLoader",message,UIResourceStringLoaderLoggerName)
#define UIResourceStringLoader_LOG_INFO(message)      LOG_INFO("UIResourceStringLoader",message,UIResourceStringLoaderLoggerName)
#define UIResourceStringLoader_LOG_WARN(message)      LOG_WARN("UIResourceStringLoader",message,UIResourceStringLoaderLoggerName)
#define UIResourceStringLoader_LOG_ERROR(message)     LOG_ERROR("UIResourceStringLoader",message,UIResourceStringLoaderLoggerName)
#define UIResourceStringLoader_LOG_FATAL(message)     LOG_FATAL("UIResourceStringLoader",message,UIResourceStringLoaderLoggerName)