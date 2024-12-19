#pragma once

#include <MasterLog/LogExport.h>

static inline const char * UIComponentBaseLoggerName = "APP";
#define UICOMBASE_LOG_DEBUG(message)     LOG_DEBUG("UIComponentBase",message,UIComponentBaseLoggerName)
#define UICOMBASE_LOG_INFO(message)      LOG_INFO("UIComponentBase",message,UIComponentBaseLoggerName)
#define UICOMBASE_LOG_WARN(message)      LOG_WARN("UIComponentBase",message,UIComponentBaseLoggerName)
#define UICOMBASE_LOG_ERROR(message)     LOG_ERROR("UIComponentBase",message,UIComponentBaseLoggerName)
#define UICOMBASE_LOG_FATAL(message)     LOG_FATAL("UIComponentBase",message,UIComponentBaseLoggerName)