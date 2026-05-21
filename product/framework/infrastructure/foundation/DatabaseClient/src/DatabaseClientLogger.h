#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* FrameworkLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* DatabaseClientLoggerTag = "DatabaseClient";

#define DBCLIENT_LOG_DEBUG(message)  UCF_LOG_DEBUG(DatabaseClientLoggerTag, message, FrameworkLoggerName)
#define DBCLIENT_LOG_INFO(message)   UCF_LOG_INFO(DatabaseClientLoggerTag, message, FrameworkLoggerName)
#define DBCLIENT_LOG_WARN(message)   UCF_LOG_WARN(DatabaseClientLoggerTag, message, FrameworkLoggerName)
#define DBCLIENT_LOG_ERROR(message)  UCF_LOG_ERROR(DatabaseClientLoggerTag, message, FrameworkLoggerName)
#define DBCLIENT_LOG_FATAL(message)  UCF_LOG_FATAL(DatabaseClientLoggerTag, message, FrameworkLoggerName)
