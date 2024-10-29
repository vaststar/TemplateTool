#pragma once

#include <MasterLog/LogExport.h>

static inline const char * FrameworkLoggerName = "APP";
#define CORE_LOG_DEBUG(message)     LOG_DEBUG("CoreFramework",message,FrameworkLoggerName)
#define CORE_LOG_INFO(message)      LOG_INFO("CoreFramework",message,FrameworkLoggerName)
#define CORE_LOG_WARN(message)      LOG_WARN("CoreFramework",message,FrameworkLoggerName)
#define CORE_LOG_ERROR(message)     LOG_ERROR("CoreFramework",message,FrameworkLoggerName)
#define CORE_LOG_FATAL(message)     LOG_FATAL("CoreFramework",message,FrameworkLoggerName)