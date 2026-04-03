#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline constexpr auto LOGGER_NAME = ucf::utilities::kAppLoggerName;
static inline constexpr auto LOGGER_TAG = "UpgradeService";

#define UPGRADE_LOG_DEBUG(message)     UCF_LOG_DEBUG(LOGGER_TAG, message, LOGGER_NAME)
#define UPGRADE_LOG_INFO(message)      UCF_LOG_INFO(LOGGER_TAG, message, LOGGER_NAME)
#define UPGRADE_LOG_WARN(message)      UCF_LOG_WARN(LOGGER_TAG, message, LOGGER_NAME)
#define UPGRADE_LOG_ERROR(message)     UCF_LOG_ERROR(LOGGER_TAG, message, LOGGER_NAME)
