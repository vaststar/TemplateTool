#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* ViewModelUtilsLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* ViewModelUtilsLoggerTag = "ViewModelUtils";

#define VIEWMODELUTILS_LOG_DEBUG(message)     UCF_LOG_DEBUG(ViewModelUtilsLoggerTag, message, ViewModelUtilsLoggerName)
#define VIEWMODELUTILS_LOG_INFO(message)      UCF_LOG_INFO(ViewModelUtilsLoggerTag, message, ViewModelUtilsLoggerName)
#define VIEWMODELUTILS_LOG_WARN(message)      UCF_LOG_WARN(ViewModelUtilsLoggerTag, message, ViewModelUtilsLoggerName)
#define VIEWMODELUTILS_LOG_ERROR(message)     UCF_LOG_ERROR(ViewModelUtilsLoggerTag, message, ViewModelUtilsLoggerName)
#define VIEWMODELUTILS_LOG_FATAL(message)     UCF_LOG_FATAL(ViewModelUtilsLoggerTag, message, ViewModelUtilsLoggerName)
