#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* ProcessBridgeLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* ProcessBridgeLoggerTag = "ProcessBridge";

#define PB_LOG_DEBUG(message)  UCF_LOG_DEBUG(ProcessBridgeLoggerTag, message, ProcessBridgeLoggerName)
#define PB_LOG_INFO(message)   UCF_LOG_INFO(ProcessBridgeLoggerTag, message, ProcessBridgeLoggerName)
#define PB_LOG_WARN(message)   UCF_LOG_WARN(ProcessBridgeLoggerTag, message, ProcessBridgeLoggerName)
#define PB_LOG_ERROR(message)  UCF_LOG_ERROR(ProcessBridgeLoggerTag, message, ProcessBridgeLoggerName)
