#pragma once

#include <ucf/utilities/LoggerUtils/LoggerExport.h>

static inline const char* TrayViewModelLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* TrayViewModelLoggerTag = "TrayViewModel";

#define TRAY_VIEW_MODEL_LOG_DEBUG(message) UCF_LOG_DEBUG(TrayViewModelLoggerTag, message, TrayViewModelLoggerName)
#define TRAY_VIEW_MODEL_LOG_INFO(message) UCF_LOG_INFO(TrayViewModelLoggerTag, message, TrayViewModelLoggerName)
#define TRAY_VIEW_MODEL_LOG_WARN(message) UCF_LOG_WARN(TrayViewModelLoggerTag, message, TrayViewModelLoggerName)
#define TRAY_VIEW_MODEL_LOG_ERROR(message) UCF_LOG_ERROR(TrayViewModelLoggerTag, message, TrayViewModelLoggerName)
#define TRAY_VIEW_MODEL_LOG_FATAL(message) UCF_LOG_FATAL(TrayViewModelLoggerTag, message, TrayViewModelLoggerName)
