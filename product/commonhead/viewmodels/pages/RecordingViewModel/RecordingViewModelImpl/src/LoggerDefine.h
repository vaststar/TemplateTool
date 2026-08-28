#pragma once

#include <ucf/utilities/LoggerUtils/LoggerExport.h>

static inline const char* RecordingViewModelLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* RecordingViewModelLoggerTag = "RecordingViewModel";

#define RECORDING_VIEW_MODEL_LOG_DEBUG(message) UCF_LOG_DEBUG(RecordingViewModelLoggerTag, message, RecordingViewModelLoggerName)
#define RECORDING_VIEW_MODEL_LOG_INFO(message) UCF_LOG_INFO(RecordingViewModelLoggerTag, message, RecordingViewModelLoggerName)
#define RECORDING_VIEW_MODEL_LOG_WARN(message) UCF_LOG_WARN(RecordingViewModelLoggerTag, message, RecordingViewModelLoggerName)
#define RECORDING_VIEW_MODEL_LOG_ERROR(message) UCF_LOG_ERROR(RecordingViewModelLoggerTag, message, RecordingViewModelLoggerName)
#define RECORDING_VIEW_MODEL_LOG_FATAL(message) UCF_LOG_FATAL(RecordingViewModelLoggerTag, message, RecordingViewModelLoggerName)
