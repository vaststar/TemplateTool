#pragma once

#include <MasterLog/LogExport.h>

static inline const char * UIResourceLoaderManagerLoggerName = "APP";
#define UIResourceLoaderManager_LOG_DEBUG(message)     LOG_DEBUG("UIResourceLoaderManager",message,UIResourceLoaderManagerLoggerName)
#define UIResourceLoaderManager_LOG_INFO(message)      LOG_INFO("UIResourceLoaderManager",message,UIResourceLoaderManagerLoggerName)
#define UIResourceLoaderManager_LOG_WARN(message)      LOG_WARN("UIResourceLoaderManager",message,UIResourceLoaderManagerLoggerName)
#define UIResourceLoaderManager_LOG_ERROR(message)     LOG_ERROR("UIResourceLoaderManager",message,UIResourceLoaderManagerLoggerName)
#define UIResourceLoaderManager_LOG_FATAL(message)     LOG_FATAL("UIResourceLoaderManager",message,UIResourceLoaderManagerLoggerName)