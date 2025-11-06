#pragma once

#include <MasterLog/LogExport.h>

static inline const char * UIManagerLoggerName = "APP";
#define UIManager_LOG_DEBUG(message)     LOG_DEBUG("UIManager",message,UIManagerLoggerName)
#define UIManager_LOG_INFO(message)      LOG_INFO("UIManager",message,UIManagerLoggerName)
#define UIManager_LOG_WARN(message)      LOG_WARN("UIManager",message,UIManagerLoggerName)
#define UIManager_LOG_ERROR(message)     LOG_ERROR("UIManager",message,UIManagerLoggerName)
#define UIManager_LOG_FATAL(message)     LOG_FATAL("UIManager",message,UIManagerLoggerName)
