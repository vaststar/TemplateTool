#pragma once

#include <MasterLog/LogExport.h>

static inline const char * UIUtilitiesLoggerName = "APP";
#define UIUtilities_LOG_DEBUG(message)     LOG_DEBUG("UIUtilities",message,UIUtilitiesLoggerName)
#define UIUtilities_LOG_INFO(message)      LOG_INFO("UIUtilities",message,UIUtilitiesLoggerName)
#define UIUtilities_LOG_WARN(message)      LOG_WARN("UIUtilities",message,UIUtilitiesLoggerName)
#define UIUtilities_LOG_ERROR(message)     LOG_ERROR("UIUtilities",message,UIUtilitiesLoggerName)
#define UIUtilities_LOG_FATAL(message)     LOG_FATAL("UIUtilities",message,UIUtilitiesLoggerName)