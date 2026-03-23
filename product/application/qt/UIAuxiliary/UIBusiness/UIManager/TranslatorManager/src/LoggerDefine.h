#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* TranslatorManagerLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* TranslatorManagerLoggerTag = "TranslatorManager";

#define TranslatorManager_LOG_DEBUG(message)     UCF_LOG_DEBUG(TranslatorManagerLoggerTag, message, TranslatorManagerLoggerName)
#define TranslatorManager_LOG_INFO(message)      UCF_LOG_INFO(TranslatorManagerLoggerTag, message, TranslatorManagerLoggerName)
#define TranslatorManager_LOG_WARN(message)      UCF_LOG_WARN(TranslatorManagerLoggerTag, message, TranslatorManagerLoggerName)
#define TranslatorManager_LOG_ERROR(message)     UCF_LOG_ERROR(TranslatorManagerLoggerTag, message, TranslatorManagerLoggerName)
#define TranslatorManager_LOG_FATAL(message)     UCF_LOG_FATAL(TranslatorManagerLoggerTag, message, TranslatorManagerLoggerName)
