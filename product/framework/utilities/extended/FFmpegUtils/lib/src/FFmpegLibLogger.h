#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* FFmpegLibLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* FFmpegLibLoggerTag = "FFmpegLib";

#define FFL_LOG_DEBUG(message)  UCF_LOG_DEBUG(FFmpegLibLoggerTag, message, FFmpegLibLoggerName)
#define FFL_LOG_INFO(message)   UCF_LOG_INFO(FFmpegLibLoggerTag, message, FFmpegLibLoggerName)
#define FFL_LOG_WARN(message)   UCF_LOG_WARN(FFmpegLibLoggerTag, message, FFmpegLibLoggerName)
#define FFL_LOG_ERROR(message)  UCF_LOG_ERROR(FFmpegLibLoggerTag, message, FFmpegLibLoggerName)
