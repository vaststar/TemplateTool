#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* ResourceColorLoaderLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* ResourceColorLoaderLoggerTag = "ResourceColorLoader";

#define RESOURCE_COLOR_LOADER_LOG_DEBUG(message) UCF_LOG_DEBUG(ResourceColorLoaderLoggerTag, message, ResourceColorLoaderLoggerName)
#define RESOURCE_COLOR_LOADER_LOG_WARN(message) UCF_LOG_WARN(ResourceColorLoaderLoggerTag, message, ResourceColorLoaderLoggerName)
