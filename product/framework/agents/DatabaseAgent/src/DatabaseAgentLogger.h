#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* FrameworkLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* DatabaseAgentLoggerTag = "DatabaseAgent";

#define DBAGENT_LOG_DEBUG(message)  UCF_LOG_DEBUG(DatabaseAgentLoggerTag, message, FrameworkLoggerName)
#define DBAGENT_LOG_INFO(message)   UCF_LOG_INFO(DatabaseAgentLoggerTag, message, FrameworkLoggerName)
#define DBAGENT_LOG_WARN(message)   UCF_LOG_WARN(DatabaseAgentLoggerTag, message, FrameworkLoggerName)
#define DBAGENT_LOG_ERROR(message)  UCF_LOG_ERROR(DatabaseAgentLoggerTag, message, FrameworkLoggerName)
#define DBAGENT_LOG_FATAL(message)  UCF_LOG_FATAL(DatabaseAgentLoggerTag, message, FrameworkLoggerName)
