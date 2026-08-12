#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* MainWindowViewModelLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* MainWindowViewModelLoggerTag = "MainWindowViewModel";

#define MAIN_WINDOW_VIEW_MODEL_LOG_DEBUG(message) UCF_LOG_DEBUG(MainWindowViewModelLoggerTag, message, MainWindowViewModelLoggerName)
#define MAIN_WINDOW_VIEW_MODEL_LOG_INFO(message) UCF_LOG_INFO(MainWindowViewModelLoggerTag, message, MainWindowViewModelLoggerName)
#define MAIN_WINDOW_VIEW_MODEL_LOG_WARN(message) UCF_LOG_WARN(MainWindowViewModelLoggerTag, message, MainWindowViewModelLoggerName)
#define MAIN_WINDOW_VIEW_MODEL_LOG_ERROR(message) UCF_LOG_ERROR(MainWindowViewModelLoggerTag, message, MainWindowViewModelLoggerName)
#define MAIN_WINDOW_VIEW_MODEL_LOG_FATAL(message) UCF_LOG_FATAL(MainWindowViewModelLoggerTag, message, MainWindowViewModelLoggerName)
