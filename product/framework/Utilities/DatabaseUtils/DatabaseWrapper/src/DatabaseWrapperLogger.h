#pragma once

#include <MasterLog/LogExport.h>

//for coreframework
static inline const char * FrameworkLoggerName = "APP";
#define DBWRAPPER_LOG_DEBUG(message)     LOG_DEBUG("DatabaseWrapper",message,FrameworkLoggerName)
#define DBWRAPPER_LOG_INFO(message)      LOG_INFO("DatabaseWrapper",message,FrameworkLoggerName)
#define DBWRAPPER_LOG_WARN(message)      LOG_WARN("DatabaseWrapper",message,FrameworkLoggerName)
#define DBWRAPPER_LOG_ERROR(message)     LOG_ERROR("DatabaseWrapper",message,FrameworkLoggerName)
#define DBWRAPPER_LOG_FATAL(message)     LOG_FATAL("DatabaseWrapper",message,FrameworkLoggerName)