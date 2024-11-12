#pragma once

#include <MasterLog/LogExport.h>

static inline const char * UICoreLoggerName = "APP";
#define UICore_LOG_DEBUG(message)     LOG_DEBUG("UICore",message,UICoreLoggerName)
#define UICore_LOG_INFO(message)      LOG_INFO("UICore",message,UICoreLoggerName)
#define UICore_LOG_WARN(message)      LOG_WARN("UICore",message,UICoreLoggerName)
#define UICore_LOG_ERROR(message)     LOG_ERROR("UICore",message,UICoreLoggerName)
#define UICore_LOG_FATAL(message)     LOG_FATAL("UICore",message,UICoreLoggerName)