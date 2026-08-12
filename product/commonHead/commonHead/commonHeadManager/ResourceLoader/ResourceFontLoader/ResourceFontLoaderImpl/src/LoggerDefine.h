#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* ResourceFontLoaderLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* ResourceFontLoaderLoggerTag = "ResourceFontLoader";

#define RESOURCE_FONT_LOADER_LOG_DEBUG(message) UCF_LOG_DEBUG(ResourceFontLoaderLoggerTag, message, ResourceFontLoaderLoggerName)
#define RESOURCE_FONT_LOADER_LOG_WARN(message) UCF_LOG_WARN(ResourceFontLoaderLoggerTag, message, ResourceFontLoaderLoggerName)
