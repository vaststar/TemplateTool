#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* MiniAppListViewModelLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* MiniAppListViewModelLoggerTag = "MiniAppListViewModel";

#define MINI_APP_LIST_VIEW_MODEL_LOG_DEBUG(message) UCF_LOG_DEBUG(MiniAppListViewModelLoggerTag, message, MiniAppListViewModelLoggerName)
#define MINI_APP_LIST_VIEW_MODEL_LOG_INFO(message) UCF_LOG_INFO(MiniAppListViewModelLoggerTag, message, MiniAppListViewModelLoggerName)
#define MINI_APP_LIST_VIEW_MODEL_LOG_WARN(message) UCF_LOG_WARN(MiniAppListViewModelLoggerTag, message, MiniAppListViewModelLoggerName)
#define MINI_APP_LIST_VIEW_MODEL_LOG_ERROR(message) UCF_LOG_ERROR(MiniAppListViewModelLoggerTag, message, MiniAppListViewModelLoggerName)
#define MINI_APP_LIST_VIEW_MODEL_LOG_FATAL(message) UCF_LOG_FATAL(MiniAppListViewModelLoggerTag, message, MiniAppListViewModelLoggerName)
