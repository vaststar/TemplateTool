#pragma once

#include <ucf/utilities/LoggerUtils/LoggerExport.h>

static inline const char* ToolsViewModelLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* ToolsViewModelLoggerTag = "ToolsViewModel";

#define TOOLS_VIEW_MODEL_LOG_DEBUG(message) UCF_LOG_DEBUG(ToolsViewModelLoggerTag, message, ToolsViewModelLoggerName)
#define TOOLS_VIEW_MODEL_LOG_INFO(message) UCF_LOG_INFO(ToolsViewModelLoggerTag, message, ToolsViewModelLoggerName)
#define TOOLS_VIEW_MODEL_LOG_WARN(message) UCF_LOG_WARN(ToolsViewModelLoggerTag, message, ToolsViewModelLoggerName)
#define TOOLS_VIEW_MODEL_LOG_ERROR(message) UCF_LOG_ERROR(ToolsViewModelLoggerTag, message, ToolsViewModelLoggerName)
#define TOOLS_VIEW_MODEL_LOG_FATAL(message) UCF_LOG_FATAL(ToolsViewModelLoggerTag, message, ToolsViewModelLoggerName)
