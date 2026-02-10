#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* UIFabricationLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* UIFabricationLoggerTag = "UIFabrication";

#define UIFabrication_LOG_DEBUG(message)     UCF_LOG_DEBUG(UIFabricationLoggerTag, message, UIFabricationLoggerName)
#define UIFabrication_LOG_INFO(message)      UCF_LOG_INFO(UIFabricationLoggerTag, message, UIFabricationLoggerName)
#define UIFabrication_LOG_WARN(message)      UCF_LOG_WARN(UIFabricationLoggerTag, message, UIFabricationLoggerName)
#define UIFabrication_LOG_ERROR(message)     UCF_LOG_ERROR(UIFabricationLoggerTag, message, UIFabricationLoggerName)
#define UIFabrication_LOG_FATAL(message)     UCF_LOG_FATAL(UIFabricationLoggerTag, message, UIFabricationLoggerName)