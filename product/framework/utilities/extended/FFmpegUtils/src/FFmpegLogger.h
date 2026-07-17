#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* FFmpegLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* FFmpegLoggerTag = "FFmpeg";

#define FF_LOG_DEBUG(message)  UCF_LOG_DEBUG(FFmpegLoggerTag, message, FFmpegLoggerName)
#define FF_LOG_INFO(message)   UCF_LOG_INFO(FFmpegLoggerTag, message, FFmpegLoggerName)
#define FF_LOG_WARN(message)   UCF_LOG_WARN(FFmpegLoggerTag, message, FFmpegLoggerName)
#define FF_LOG_ERROR(message)  UCF_LOG_ERROR(FFmpegLoggerTag, message, FFmpegLoggerName)
