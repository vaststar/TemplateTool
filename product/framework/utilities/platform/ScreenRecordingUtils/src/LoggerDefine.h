#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* ScreenRecordingUtilsLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* ScreenRecordingUtilsLoggerTag = "ScreenRecordingUtils";

#define SRU_LOG_DEBUG(message) UCF_LOG_DEBUG(ScreenRecordingUtilsLoggerTag, message, ScreenRecordingUtilsLoggerName)
#define SRU_LOG_INFO(message)  UCF_LOG_INFO(ScreenRecordingUtilsLoggerTag, message, ScreenRecordingUtilsLoggerName)
#define SRU_LOG_WARN(message)  UCF_LOG_WARN(ScreenRecordingUtilsLoggerTag, message, ScreenRecordingUtilsLoggerName)
#define SRU_LOG_ERROR(message) UCF_LOG_ERROR(ScreenRecordingUtilsLoggerTag, message, ScreenRecordingUtilsLoggerName)
