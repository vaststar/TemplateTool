#pragma once

#include <MasterLog/LogExport.h>

static inline const char * UIDataLoggerName = "APP";
#define UIData_LOG_DEBUG(message)     LOG_DEBUG("UIData",message,UIDataLoggerName)
#define UIData_LOG_INFO(message)      LOG_INFO("UIData",message,UIDataLoggerName)
#define UIData_LOG_WARN(message)      LOG_WARN("UIData",message,UIDataLoggerName)
#define UIData_LOG_ERROR(message)     LOG_ERROR("UIData",message,UIDataLoggerName)
#define UIData_LOG_FATAL(message)     LOG_FATAL("UIData",message,UIDataLoggerName)