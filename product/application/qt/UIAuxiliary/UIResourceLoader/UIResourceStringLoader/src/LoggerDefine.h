#pragma once

#include <MasterLog/LogExport.h>

static inline const char * UIResourceeStringLoaderLoggerName = "APP";
#define UIResourceeStringLoader_LOG_DEBUG(message)     LOG_DEBUG("UIResourceeStringLoader",message,UIResourceeStringLoaderLoggerName)
#define UIResourceeStringLoader_LOG_INFO(message)      LOG_INFO("UIResourceeStringLoader",message,UIResourceeStringLoaderLoggerName)
#define UIResourceeStringLoader_LOG_WARN(message)      LOG_WARN("UIResourceeStringLoader",message,UIResourceeStringLoaderLoggerName)
#define UIResourceeStringLoader_LOG_ERROR(message)     LOG_ERROR("UIResourceeStringLoader",message,UIResourceeStringLoaderLoggerName)
#define UIResourceeStringLoader_LOG_FATAL(message)     LOG_FATAL("UIResourceeStringLoader",message,UIResourceeStringLoaderLoggerName)