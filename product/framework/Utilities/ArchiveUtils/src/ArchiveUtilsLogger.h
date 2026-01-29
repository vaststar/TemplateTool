#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerUtils.h>

#define ARCHIVE_LOG_TRACE(msg)   UCF_LOG_TRACE("ArchiveUtils", msg)
#define ARCHIVE_LOG_DEBUG(msg)   UCF_LOG_DEBUG("ArchiveUtils", msg)
#define ARCHIVE_LOG_INFO(msg)    UCF_LOG_INFO("ArchiveUtils", msg)
#define ARCHIVE_LOG_WARNING(msg) UCF_LOG_WARNING("ArchiveUtils", msg)
#define ARCHIVE_LOG_ERROR(msg)   UCF_LOG_ERROR("ArchiveUtils", msg)
#define ARCHIVE_LOG_FATAL(msg)   UCF_LOG_FATAL("ArchiveUtils", msg)
