#pragma once

#include <ucf/utilities/LoggerUtils/LoggerExport.h>

static inline const char* ResourceLoaderLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* ResourceLoaderLoggerTag = "ResourceLoader";

#define RESOURCE_LOADER_LOG_DEBUG(message) UCF_LOG_DEBUG(ResourceLoaderLoggerTag, message, ResourceLoaderLoggerName)
#define RESOURCE_LOADER_LOG_INFO(message) UCF_LOG_INFO(ResourceLoaderLoggerTag, message, ResourceLoaderLoggerName)
#define RESOURCE_LOADER_LOG_WARN(message) UCF_LOG_WARN(ResourceLoaderLoggerTag, message, ResourceLoaderLoggerName)
