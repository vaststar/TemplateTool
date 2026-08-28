#pragma once

#include <ucf/utilities/LoggerUtils/LoggerExport.h>

static inline const char* UuidToolViewModelLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* UuidToolViewModelLoggerTag = "UuidToolViewModel";

#define UUID_TOOL_VIEW_MODEL_LOG_DEBUG(message) UCF_LOG_DEBUG(UuidToolViewModelLoggerTag, message, UuidToolViewModelLoggerName)
#define UUID_TOOL_VIEW_MODEL_LOG_INFO(message) UCF_LOG_INFO(UuidToolViewModelLoggerTag, message, UuidToolViewModelLoggerName)
#define UUID_TOOL_VIEW_MODEL_LOG_WARN(message) UCF_LOG_WARN(UuidToolViewModelLoggerTag, message, UuidToolViewModelLoggerName)
#define UUID_TOOL_VIEW_MODEL_LOG_ERROR(message) UCF_LOG_ERROR(UuidToolViewModelLoggerTag, message, UuidToolViewModelLoggerName)
#define UUID_TOOL_VIEW_MODEL_LOG_FATAL(message) UCF_LOG_FATAL(UuidToolViewModelLoggerTag, message, UuidToolViewModelLoggerName)
