#pragma once

#include <ucf/utilities/LoggerUtils/LoggerExport.h>

static inline const char* JsonToolViewModelLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* JsonToolViewModelLoggerTag = "JsonToolViewModel";

#define JSON_TOOL_VIEW_MODEL_LOG_DEBUG(message) UCF_LOG_DEBUG(JsonToolViewModelLoggerTag, message, JsonToolViewModelLoggerName)
#define JSON_TOOL_VIEW_MODEL_LOG_INFO(message) UCF_LOG_INFO(JsonToolViewModelLoggerTag, message, JsonToolViewModelLoggerName)
#define JSON_TOOL_VIEW_MODEL_LOG_WARN(message) UCF_LOG_WARN(JsonToolViewModelLoggerTag, message, JsonToolViewModelLoggerName)
#define JSON_TOOL_VIEW_MODEL_LOG_ERROR(message) UCF_LOG_ERROR(JsonToolViewModelLoggerTag, message, JsonToolViewModelLoggerName)
#define JSON_TOOL_VIEW_MODEL_LOG_FATAL(message) UCF_LOG_FATAL(JsonToolViewModelLoggerTag, message, JsonToolViewModelLoggerName)
