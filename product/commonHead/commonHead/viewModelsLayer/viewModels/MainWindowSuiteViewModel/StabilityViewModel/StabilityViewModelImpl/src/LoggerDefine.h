#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* StabilityViewModelLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* StabilityViewModelLoggerTag = "StabilityViewModel";

#define STABILITY_VIEW_MODEL_LOG_DEBUG(message) UCF_LOG_DEBUG(StabilityViewModelLoggerTag, message, StabilityViewModelLoggerName)
#define STABILITY_VIEW_MODEL_LOG_INFO(message) UCF_LOG_INFO(StabilityViewModelLoggerTag, message, StabilityViewModelLoggerName)
#define STABILITY_VIEW_MODEL_LOG_WARN(message) UCF_LOG_WARN(StabilityViewModelLoggerTag, message, StabilityViewModelLoggerName)
#define STABILITY_VIEW_MODEL_LOG_ERROR(message) UCF_LOG_ERROR(StabilityViewModelLoggerTag, message, StabilityViewModelLoggerName)
#define STABILITY_VIEW_MODEL_LOG_FATAL(message) UCF_LOG_FATAL(StabilityViewModelLoggerTag, message, StabilityViewModelLoggerName)
