#pragma once

#include <ucf/utilities/LoggerUtils/LoggerExport.h>

static inline const char* Base64ToolViewModelLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* Base64ToolViewModelLoggerTag = "Base64ToolViewModel";

#define BASE64_TOOL_VIEW_MODEL_LOG_DEBUG(message) UCF_LOG_DEBUG(Base64ToolViewModelLoggerTag, message, Base64ToolViewModelLoggerName)
#define BASE64_TOOL_VIEW_MODEL_LOG_INFO(message) UCF_LOG_INFO(Base64ToolViewModelLoggerTag, message, Base64ToolViewModelLoggerName)
#define BASE64_TOOL_VIEW_MODEL_LOG_WARN(message) UCF_LOG_WARN(Base64ToolViewModelLoggerTag, message, Base64ToolViewModelLoggerName)
#define BASE64_TOOL_VIEW_MODEL_LOG_ERROR(message) UCF_LOG_ERROR(Base64ToolViewModelLoggerTag, message, Base64ToolViewModelLoggerName)
#define BASE64_TOOL_VIEW_MODEL_LOG_FATAL(message) UCF_LOG_FATAL(Base64ToolViewModelLoggerTag, message, Base64ToolViewModelLoggerName)
