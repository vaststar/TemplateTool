#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* UIResourceAssetLoaderLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* UIResourceAssetLoaderLoggerTag = "UIResourceAssetLoader";

#define UIResourceAssetLoader_LOG_DEBUG(message)     UCF_LOG_DEBUG(UIResourceAssetLoaderLoggerTag, message, UIResourceAssetLoaderLoggerName)
#define UIResourceAssetLoader_LOG_INFO(message)      UCF_LOG_INFO(UIResourceAssetLoaderLoggerTag, message, UIResourceAssetLoaderLoggerName)
#define UIResourceAssetLoader_LOG_WARN(message)      UCF_LOG_WARN(UIResourceAssetLoaderLoggerTag, message, UIResourceAssetLoaderLoggerName)
#define UIResourceAssetLoader_LOG_ERROR(message)     UCF_LOG_ERROR(UIResourceAssetLoaderLoggerTag, message, UIResourceAssetLoaderLoggerName)
#define UIResourceAssetLoader_LOG_FATAL(message)     UCF_LOG_FATAL(UIResourceAssetLoaderLoggerTag, message, UIResourceAssetLoaderLoggerName)