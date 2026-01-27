#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* UIResourceLoaderManagerLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* UIResourceLoaderManagerLoggerTag = "UIResourceLoaderManager";

#define UIResourceLoaderManager_LOG_DEBUG(message)     UCF_LOG_DEBUG(UIResourceLoaderManagerLoggerTag, message, UIResourceLoaderManagerLoggerName)
#define UIResourceLoaderManager_LOG_INFO(message)      UCF_LOG_INFO(UIResourceLoaderManagerLoggerTag, message, UIResourceLoaderManagerLoggerName)
#define UIResourceLoaderManager_LOG_WARN(message)      UCF_LOG_WARN(UIResourceLoaderManagerLoggerTag, message, UIResourceLoaderManagerLoggerName)
#define UIResourceLoaderManager_LOG_ERROR(message)     UCF_LOG_ERROR(UIResourceLoaderManagerLoggerTag, message, UIResourceLoaderManagerLoggerName)
#define UIResourceLoaderManager_LOG_FATAL(message)     UCF_LOG_FATAL(UIResourceLoaderManagerLoggerTag, message, UIResourceLoaderManagerLoggerName)