#pragma once

#include "MasterLog/LogExport.h"

//for coreframework
static inline const char * FrameworkLoggerName = "APP";

#define COMMONHEAD_LOG_DEBUG(message)     LOG_DEBUG("CommonHead",message,FrameworkLoggerName)
#define COMMONHEAD_LOG_INFO(message)      LOG_INFO("CommonHead",message,FrameworkLoggerName)
#define COMMONHEAD_LOG_WARN(message)      LOG_WARN("CommonHead",message,FrameworkLoggerName)
#define COMMONHEAD_LOG_ERROR(message)     LOG_ERROR("CommonHead",message,FrameworkLoggerName)
#define COMMONHEAD_LOG_FATAL(message)     LOG_FATAL("CommonHead",message,FrameworkLoggerName)