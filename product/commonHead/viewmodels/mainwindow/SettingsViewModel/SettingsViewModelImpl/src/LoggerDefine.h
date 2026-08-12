#pragma once

#include <ucf/utilities/LoggerUtils/LoggerExport.h>

static inline const char* SettingsViewModelLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* SettingsViewModelLoggerTag = "SettingsViewModel";

#define SETTINGS_VIEW_MODEL_LOG_DEBUG(message) UCF_LOG_DEBUG(SettingsViewModelLoggerTag, message, SettingsViewModelLoggerName)
#define SETTINGS_VIEW_MODEL_LOG_INFO(message) UCF_LOG_INFO(SettingsViewModelLoggerTag, message, SettingsViewModelLoggerName)
#define SETTINGS_VIEW_MODEL_LOG_WARN(message) UCF_LOG_WARN(SettingsViewModelLoggerTag, message, SettingsViewModelLoggerName)
#define SETTINGS_VIEW_MODEL_LOG_ERROR(message) UCF_LOG_ERROR(SettingsViewModelLoggerTag, message, SettingsViewModelLoggerName)
#define SETTINGS_VIEW_MODEL_LOG_FATAL(message) UCF_LOG_FATAL(SettingsViewModelLoggerTag, message, SettingsViewModelLoggerName)
