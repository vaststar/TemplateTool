#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* ScreenRecordingAgentLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* ScreenRecordingAgentLoggerTag = "ScreenRecordingAgent";

#define SRA_LOG_DEBUG(message)  UCF_LOG_DEBUG(ScreenRecordingAgentLoggerTag, message, ScreenRecordingAgentLoggerName)
#define SRA_LOG_INFO(message)   UCF_LOG_INFO(ScreenRecordingAgentLoggerTag, message, ScreenRecordingAgentLoggerName)
#define SRA_LOG_WARN(message)   UCF_LOG_WARN(ScreenRecordingAgentLoggerTag, message, ScreenRecordingAgentLoggerName)
#define SRA_LOG_ERROR(message)  UCF_LOG_ERROR(ScreenRecordingAgentLoggerTag, message, ScreenRecordingAgentLoggerName)
