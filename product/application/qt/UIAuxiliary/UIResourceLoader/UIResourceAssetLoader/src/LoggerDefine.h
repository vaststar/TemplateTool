#pragma once

#include <MasterLog/LogExport.h>

static inline const char * UIResourceAssetLoaderLoggerName = "APP";
#define UIResourceAssetLoader_LOG_DEBUG(message)     LOG_DEBUG("UIResourceAssetLoader",message,UIResourceAssetLoaderLoggerName)
#define UIResourceAssetLoader_LOG_INFO(message)      LOG_INFO("UIResourceAssetLoader",message,UIResourceAssetLoaderLoggerName)
#define UIResourceAssetLoader_LOG_WARN(message)      LOG_WARN("UIResourceAssetLoader",message,UIResourceAssetLoaderLoggerName)
#define UIResourceAssetLoader_LOG_ERROR(message)     LOG_ERROR("UIResourceAssetLoader",message,UIResourceAssetLoaderLoggerName)
#define UIResourceAssetLoader_LOG_FATAL(message)     LOG_FATAL("UIResourceAssetLoader",message,UIResourceAssetLoaderLoggerName)