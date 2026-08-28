#pragma once

#include <ucf/utilities/LoggerUtils/LoggerExport.h>

static inline const char* MiniAppRuntimeAgentLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* MiniAppRuntimeAgentLoggerTag = "MiniAppRuntimeAgent";

#define MINI_APP_RUNTIME_AGENT_LOG_DEBUG(message) UCF_LOG_DEBUG(MiniAppRuntimeAgentLoggerTag, message, MiniAppRuntimeAgentLoggerName)
#define MINI_APP_RUNTIME_AGENT_LOG_INFO(message) UCF_LOG_INFO(MiniAppRuntimeAgentLoggerTag, message, MiniAppRuntimeAgentLoggerName)
#define MINI_APP_RUNTIME_AGENT_LOG_WARN(message) UCF_LOG_WARN(MiniAppRuntimeAgentLoggerTag, message, MiniAppRuntimeAgentLoggerName)
#define MINI_APP_RUNTIME_AGENT_LOG_ERROR(message) UCF_LOG_ERROR(MiniAppRuntimeAgentLoggerTag, message, MiniAppRuntimeAgentLoggerName)
#define MINI_APP_RUNTIME_AGENT_LOG_FATAL(message) UCF_LOG_FATAL(MiniAppRuntimeAgentLoggerTag, message, MiniAppRuntimeAgentLoggerName)
