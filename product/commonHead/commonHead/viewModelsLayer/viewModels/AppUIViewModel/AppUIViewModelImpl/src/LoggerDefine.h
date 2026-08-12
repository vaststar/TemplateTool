#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* AppUIViewModelLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* AppUIViewModelLoggerTag = "AppUIViewModel";

#define APP_UI_VIEW_MODEL_LOG_DEBUG(message) UCF_LOG_DEBUG(AppUIViewModelLoggerTag, message, AppUIViewModelLoggerName)
#define APP_UI_VIEW_MODEL_LOG_INFO(message) UCF_LOG_INFO(AppUIViewModelLoggerTag, message, AppUIViewModelLoggerName)
#define APP_UI_VIEW_MODEL_LOG_WARN(message) UCF_LOG_WARN(AppUIViewModelLoggerTag, message, AppUIViewModelLoggerName)
#define APP_UI_VIEW_MODEL_LOG_ERROR(message) UCF_LOG_ERROR(AppUIViewModelLoggerTag, message, AppUIViewModelLoggerName)
#define APP_UI_VIEW_MODEL_LOG_FATAL(message) UCF_LOG_FATAL(AppUIViewModelLoggerTag, message, AppUIViewModelLoggerName)
