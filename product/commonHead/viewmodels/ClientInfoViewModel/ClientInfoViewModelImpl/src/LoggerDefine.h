#pragma once

#include <ucf/utilities/LoggerUtils/LoggerExport.h>

static inline const char* ClientInfoViewModelLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* ClientInfoViewModelLoggerTag = "ClientInfoViewModel";

#define CLIENT_INFO_VIEW_MODEL_LOG_DEBUG(message) UCF_LOG_DEBUG(ClientInfoViewModelLoggerTag, message, ClientInfoViewModelLoggerName)
#define CLIENT_INFO_VIEW_MODEL_LOG_INFO(message) UCF_LOG_INFO(ClientInfoViewModelLoggerTag, message, ClientInfoViewModelLoggerName)
#define CLIENT_INFO_VIEW_MODEL_LOG_WARN(message) UCF_LOG_WARN(ClientInfoViewModelLoggerTag, message, ClientInfoViewModelLoggerName)
#define CLIENT_INFO_VIEW_MODEL_LOG_ERROR(message) UCF_LOG_ERROR(ClientInfoViewModelLoggerTag, message, ClientInfoViewModelLoggerName)
#define CLIENT_INFO_VIEW_MODEL_LOG_FATAL(message) UCF_LOG_FATAL(ClientInfoViewModelLoggerTag, message, ClientInfoViewModelLoggerName)
