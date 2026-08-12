#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* UpgradeViewModelLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* UpgradeViewModelLoggerTag = "UpgradeViewModel";

#define UPGRADE_VIEW_MODEL_LOG_DEBUG(message) UCF_LOG_DEBUG(UpgradeViewModelLoggerTag, message, UpgradeViewModelLoggerName)
#define UPGRADE_VIEW_MODEL_LOG_INFO(message) UCF_LOG_INFO(UpgradeViewModelLoggerTag, message, UpgradeViewModelLoggerName)
#define UPGRADE_VIEW_MODEL_LOG_WARN(message) UCF_LOG_WARN(UpgradeViewModelLoggerTag, message, UpgradeViewModelLoggerName)
#define UPGRADE_VIEW_MODEL_LOG_ERROR(message) UCF_LOG_ERROR(UpgradeViewModelLoggerTag, message, UpgradeViewModelLoggerName)
#define UPGRADE_VIEW_MODEL_LOG_FATAL(message) UCF_LOG_FATAL(UpgradeViewModelLoggerTag, message, UpgradeViewModelLoggerName)
