#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* UIIPCChannelLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* UIIPCChannelLoggerTag = "UIIPCChannel";

#define UIIPCChannel_LOG_DEBUG(message)     UCF_LOG_DEBUG(UIIPCChannelLoggerTag, message, UIIPCChannelLoggerName)
#define UIIPCChannel_LOG_INFO(message)      UCF_LOG_INFO(UIIPCChannelLoggerTag, message, UIIPCChannelLoggerName)
#define UIIPCChannel_LOG_WARN(message)      UCF_LOG_WARN(UIIPCChannelLoggerTag, message, UIIPCChannelLoggerName)
#define UIIPCChannel_LOG_ERROR(message)     UCF_LOG_ERROR(UIIPCChannelLoggerTag, message, UIIPCChannelLoggerName)
#define UIIPCChannel_LOG_FATAL(message)     UCF_LOG_FATAL(UIIPCChannelLoggerTag, message, UIIPCChannelLoggerName)