#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* InvocationViewModelLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* InvocationViewModelLoggerTag = "InvocationViewModel";

#define INVOCATION_VIEW_MODEL_LOG_DEBUG(message) UCF_LOG_DEBUG(InvocationViewModelLoggerTag, message, InvocationViewModelLoggerName)
#define INVOCATION_VIEW_MODEL_LOG_INFO(message) UCF_LOG_INFO(InvocationViewModelLoggerTag, message, InvocationViewModelLoggerName)
#define INVOCATION_VIEW_MODEL_LOG_WARN(message) UCF_LOG_WARN(InvocationViewModelLoggerTag, message, InvocationViewModelLoggerName)
#define INVOCATION_VIEW_MODEL_LOG_ERROR(message) UCF_LOG_ERROR(InvocationViewModelLoggerTag, message, InvocationViewModelLoggerName)
#define INVOCATION_VIEW_MODEL_LOG_FATAL(message) UCF_LOG_FATAL(InvocationViewModelLoggerTag, message, InvocationViewModelLoggerName)
