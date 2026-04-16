#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* CompressionUtilsLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* CompressionUtilsLoggerTag = "CompressionUtils";

#define COMPRESSION_LOG_DEBUG(message)  UCF_LOG_DEBUG(CompressionUtilsLoggerTag, message, CompressionUtilsLoggerName)
#define COMPRESSION_LOG_INFO(message)   UCF_LOG_INFO(CompressionUtilsLoggerTag, message, CompressionUtilsLoggerName)
#define COMPRESSION_LOG_WARN(message)   UCF_LOG_WARN(CompressionUtilsLoggerTag, message, CompressionUtilsLoggerName)
#define COMPRESSION_LOG_ERROR(message)  UCF_LOG_ERROR(CompressionUtilsLoggerTag, message, CompressionUtilsLoggerName)
#define COMPRESSION_LOG_FATAL(message)  UCF_LOG_FATAL(CompressionUtilsLoggerTag, message, CompressionUtilsLoggerName)
