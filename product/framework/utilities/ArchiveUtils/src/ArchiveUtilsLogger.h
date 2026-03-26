#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* ArchiveUtilsLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* ArchiveUtilsLoggerTag = "ArchiveUtils";

#define ARCHIVE_LOG_DEBUG(msg)   UCF_LOG_DEBUG(ArchiveUtilsLoggerTag, msg, ArchiveUtilsLoggerName)
#define ARCHIVE_LOG_INFO(msg)    UCF_LOG_INFO(ArchiveUtilsLoggerTag, msg, ArchiveUtilsLoggerName)
#define ARCHIVE_LOG_WARNING(msg) UCF_LOG_WARN(ArchiveUtilsLoggerTag, msg, ArchiveUtilsLoggerName)
#define ARCHIVE_LOG_ERROR(msg)   UCF_LOG_ERROR(ArchiveUtilsLoggerTag, msg, ArchiveUtilsLoggerName)
#define ARCHIVE_LOG_FATAL(msg)   UCF_LOG_FATAL(ArchiveUtilsLoggerTag, msg, ArchiveUtilsLoggerName)
