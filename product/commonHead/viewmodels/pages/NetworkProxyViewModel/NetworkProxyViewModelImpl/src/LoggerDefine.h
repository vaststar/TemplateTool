#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* NetworkProxyViewModelLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* NetworkProxyViewModelLoggerTag = "NetworkProxyViewModel";

#define NETWORK_PROXY_VIEW_MODEL_LOG_DEBUG(message) UCF_LOG_DEBUG(NetworkProxyViewModelLoggerTag, message, NetworkProxyViewModelLoggerName)
#define NETWORK_PROXY_VIEW_MODEL_LOG_INFO(message) UCF_LOG_INFO(NetworkProxyViewModelLoggerTag, message, NetworkProxyViewModelLoggerName)
#define NETWORK_PROXY_VIEW_MODEL_LOG_WARN(message) UCF_LOG_WARN(NetworkProxyViewModelLoggerTag, message, NetworkProxyViewModelLoggerName)
#define NETWORK_PROXY_VIEW_MODEL_LOG_ERROR(message) UCF_LOG_ERROR(NetworkProxyViewModelLoggerTag, message, NetworkProxyViewModelLoggerName)
#define NETWORK_PROXY_VIEW_MODEL_LOG_FATAL(message) UCF_LOG_FATAL(NetworkProxyViewModelLoggerTag, message, NetworkProxyViewModelLoggerName)
