#pragma once

#include <MasterLog/LogExport.h>

//for coreframework
static inline const char * FrameworkLoggerName = "APP";
#define LIBCURL_LOG_DEBUG(message)     LOG_DEBUG("LibCurlClient",message,FrameworkLoggerName)
#define LIBCURL_LOG_INFO(message)      LOG_INFO("LibCurlClient",message,FrameworkLoggerName)
#define LIBCURL_LOG_WARN(message)      LOG_WARN("LibCurlClient",message,FrameworkLoggerName)
#define LIBCURL_LOG_ERROR(message)     LOG_ERROR("LibCurlClient",message,FrameworkLoggerName)
#define LIBCURL_LOG_FATAL(message)     LOG_FATAL("LibCurlClient",message,FrameworkLoggerName)