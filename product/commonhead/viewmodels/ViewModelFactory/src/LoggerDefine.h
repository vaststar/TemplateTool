#pragma once

#include <ucf/utilities/LoggerUtils/LoggerExport.h>

static inline const char* ViewModelFactoryLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* ViewModelFactoryLoggerTag = "ViewModelFactory";

#define VIEW_MODEL_FACTORY_LOG_DEBUG(message) UCF_LOG_DEBUG(ViewModelFactoryLoggerTag, message, ViewModelFactoryLoggerName)
#define VIEW_MODEL_FACTORY_LOG_INFO(message) UCF_LOG_INFO(ViewModelFactoryLoggerTag, message, ViewModelFactoryLoggerName)
#define VIEW_MODEL_FACTORY_LOG_WARN(message) UCF_LOG_WARN(ViewModelFactoryLoggerTag, message, ViewModelFactoryLoggerName)
#define VIEW_MODEL_FACTORY_LOG_ERROR(message) UCF_LOG_ERROR(ViewModelFactoryLoggerTag, message, ViewModelFactoryLoggerName)
#define VIEW_MODEL_FACTORY_LOG_FATAL(message) UCF_LOG_FATAL(ViewModelFactoryLoggerTag, message, ViewModelFactoryLoggerName)
