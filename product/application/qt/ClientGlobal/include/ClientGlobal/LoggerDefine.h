#pragma once

#include "LogExport.h"

static inline const char * ClientGlobalLoggerName = "APP";
#define CLIENTGLOBAL_LOG_DEBUG(message)     LOG_DEBUG("ClientGlobal",message,ClientGlobalLoggerName)
#define CLIENTGLOBAL_LOG_INFO(message)      LOG_INFO("ClientGlobal",message,ClientGlobalLoggerName)
#define CLIENTGLOBAL_LOG_WARN(message)      LOG_WARN("ClientGlobal",message,ClientGlobalLoggerName)
#define CLIENTGLOBAL_LOG_ERROR(message)     LOG_ERROR("ClientGlobal",message,ClientGlobalLoggerName)
#define CLIENTGLOBAL_LOG_FATAL(message)     LOG_FATAL("ClientGlobal",message,ClientGlobalLoggerName)