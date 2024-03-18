#pragma once

#include "LogExport.h"

//for coreframework
static inline const char * FrameworkLoggerName = "APP";
#define CORE_LOG_DEBUG(message)     LOG_DEBUG("CoreFramework",message,FrameworkLoggerName)
#define CORE_LOG_INFO(message)      LOG_INFO("CoreFramework",message,FrameworkLoggerName)
#define CORE_LOG_WARN(message)      LOG_WARN("CoreFramework",message,FrameworkLoggerName)
#define CORE_LOG_ERROR(message)     LOG_ERROR("CoreFramework",message,FrameworkLoggerName)
#define CORE_LOG_FATAL(message)     LOG_FATAL("CoreFramework",message,FrameworkLoggerName)


#define SERVICE_LOG_DEBUG(message)     LOG_DEBUG("Service",message,FrameworkLoggerName)
#define SERVICE_LOG_INFO(message)      LOG_INFO("Service",message,FrameworkLoggerName)
#define SERVICE_LOG_WARN(message)      LOG_WARN("Service",message,FrameworkLoggerName)
#define SERVICE_LOG_ERROR(message)     LOG_ERROR("Service",message,FrameworkLoggerName)
#define SERVICE_LOG_FATAL(message)     LOG_FATAL("Service",message,FrameworkLoggerName)