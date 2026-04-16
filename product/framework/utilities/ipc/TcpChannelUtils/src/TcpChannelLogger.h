#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* TcpChannelLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* TcpChannelLoggerTag = "TcpChannel";

#define TC_LOG_DEBUG(message)  UCF_LOG_DEBUG(TcpChannelLoggerTag, message, TcpChannelLoggerName)
#define TC_LOG_INFO(message)   UCF_LOG_INFO(TcpChannelLoggerTag, message, TcpChannelLoggerName)
#define TC_LOG_WARN(message)   UCF_LOG_WARN(TcpChannelLoggerTag, message, TcpChannelLoggerName)
#define TC_LOG_ERROR(message)  UCF_LOG_ERROR(TcpChannelLoggerTag, message, TcpChannelLoggerName)
