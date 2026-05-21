#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* NetworkProxyAgentLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* NetworkProxyAgentLoggerTag = "NetworkProxyAgent";

#define NPA_LOG_DEBUG(message)  UCF_LOG_DEBUG(NetworkProxyAgentLoggerTag, message, NetworkProxyAgentLoggerName)
#define NPA_LOG_INFO(message)   UCF_LOG_INFO(NetworkProxyAgentLoggerTag, message, NetworkProxyAgentLoggerName)
#define NPA_LOG_WARN(message)   UCF_LOG_WARN(NetworkProxyAgentLoggerTag, message, NetworkProxyAgentLoggerName)
#define NPA_LOG_ERROR(message)  UCF_LOG_ERROR(NetworkProxyAgentLoggerTag, message, NetworkProxyAgentLoggerName)
