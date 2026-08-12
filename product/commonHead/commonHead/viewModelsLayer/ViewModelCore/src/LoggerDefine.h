#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* ViewModelCoreLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* ViewModelCoreLoggerTag = "ViewModelCore";

#define VIEW_MODEL_CORE_LOG_DEBUG(message) UCF_LOG_DEBUG(ViewModelCoreLoggerTag, message, ViewModelCoreLoggerName)
#define VIEW_MODEL_CORE_LOG_INFO(message) UCF_LOG_INFO(ViewModelCoreLoggerTag, message, ViewModelCoreLoggerName)
#define VIEW_MODEL_CORE_LOG_WARN(message) UCF_LOG_WARN(ViewModelCoreLoggerTag, message, ViewModelCoreLoggerName)
#define VIEW_MODEL_CORE_LOG_ERROR(message) UCF_LOG_ERROR(ViewModelCoreLoggerTag, message, ViewModelCoreLoggerName)
#define VIEW_MODEL_CORE_LOG_FATAL(message) UCF_LOG_FATAL(ViewModelCoreLoggerTag, message, ViewModelCoreLoggerName)
