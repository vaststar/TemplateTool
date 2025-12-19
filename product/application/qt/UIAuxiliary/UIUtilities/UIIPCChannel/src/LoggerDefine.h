#pragma once

#include <MasterLog/LogExport.h>

static inline const char * UIIPCChannelLoggerName = "APP";
#define UIIPCChannel_LOG_DEBUG(message)     LOG_DEBUG("UIIPCChannel",message,UIIPCChannelLoggerName)
#define UIIPCChannel_LOG_INFO(message)      LOG_INFO("UIIPCChannel",message,UIIPCChannelLoggerName)
#define UIIPCChannel_LOG_WARN(message)      LOG_WARN("UIIPCChannel",message,UIIPCChannelLoggerName)
#define UIIPCChannel_LOG_ERROR(message)     LOG_ERROR("UIIPCChannel",message,UIIPCChannelLoggerName)
#define UIIPCChannel_LOG_FATAL(message)     LOG_FATAL("UIIPCChannel",message,UIIPCChannelLoggerName)