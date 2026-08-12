#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* MiniAppRuntimeViewModelLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* MiniAppRuntimeViewModelLoggerTag = "MiniAppRuntimeViewModel";

#define MINI_APP_RUNTIME_VIEW_MODEL_LOG_DEBUG(message) UCF_LOG_DEBUG(MiniAppRuntimeViewModelLoggerTag, message, MiniAppRuntimeViewModelLoggerName)
#define MINI_APP_RUNTIME_VIEW_MODEL_LOG_INFO(message) UCF_LOG_INFO(MiniAppRuntimeViewModelLoggerTag, message, MiniAppRuntimeViewModelLoggerName)
#define MINI_APP_RUNTIME_VIEW_MODEL_LOG_WARN(message) UCF_LOG_WARN(MiniAppRuntimeViewModelLoggerTag, message, MiniAppRuntimeViewModelLoggerName)
#define MINI_APP_RUNTIME_VIEW_MODEL_LOG_ERROR(message) UCF_LOG_ERROR(MiniAppRuntimeViewModelLoggerTag, message, MiniAppRuntimeViewModelLoggerName)
#define MINI_APP_RUNTIME_VIEW_MODEL_LOG_FATAL(message) UCF_LOG_FATAL(MiniAppRuntimeViewModelLoggerTag, message, MiniAppRuntimeViewModelLoggerName)
