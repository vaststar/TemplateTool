#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* ResourceAssetLoaderLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* ResourceAssetLoaderLoggerTag = "ResourceAssetLoader";

#define RESOURCE_ASSET_LOADER_LOG_DEBUG(message) UCF_LOG_DEBUG(ResourceAssetLoaderLoggerTag, message, ResourceAssetLoaderLoggerName)
#define RESOURCE_ASSET_LOADER_LOG_WARN(message) UCF_LOG_WARN(ResourceAssetLoaderLoggerTag, message, ResourceAssetLoaderLoggerName)
