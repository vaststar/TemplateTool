#pragma once

#include <MasterLog/LogExport.h>

static inline const char * AppContextLoggerName = "APP";
#define AppContext_LOG_DEBUG(message)     LOG_DEBUG("AppContext",message,AppContextLoggerName)
#define AppContext_LOG_INFO(message)      LOG_INFO("AppContext",message,AppContextLoggerName)
#define AppContext_LOG_WARN(message)      LOG_WARN("AppContext",message,AppContextLoggerName)
#define AppContext_LOG_ERROR(message)     LOG_ERROR("AppContext",message,AppContextLoggerName)
#define AppContext_LOG_FATAL(message)     LOG_FATAL("AppContext",message,AppContextLoggerName)