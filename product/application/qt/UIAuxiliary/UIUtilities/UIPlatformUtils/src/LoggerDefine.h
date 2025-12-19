#pragma once

#include <MasterLog/LogExport.h>

static inline const char * UIPlatformUtilsLoggerName = "APP";
#define UIPlatformUtils_LOG_DEBUG(message)     LOG_DEBUG("UIPlatformUtils",message,UIPlatformUtilsLoggerName)
#define UIPlatformUtils_LOG_INFO(message)      LOG_INFO("UIPlatformUtils",message,UIPlatformUtilsLoggerName)
#define UIPlatformUtils_LOG_WARN(message)      LOG_WARN("UIPlatformUtils",message,UIPlatformUtilsLoggerName)
#define UIPlatformUtils_LOG_ERROR(message)     LOG_ERROR("UIPlatformUtils",message,UIPlatformUtilsLoggerName)
#define UIPlatformUtils_LOG_FATAL(message)     LOG_FATAL("UIPlatformUtils",message,UIPlatformUtilsLoggerName)