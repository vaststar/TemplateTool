#pragma once

#include <MasterLog/LogExport.h>

static inline constexpr auto LOGGER_NAME = "APP";
#define SERVICE_LOG_DEBUG(message)     LOG_DEBUG("CommonSettingService",message,LOGGER_NAME)
#define SERVICE_LOG_INFO(message)      LOG_INFO("CommonSettingService",message,LOGGER_NAME)
#define SERVICE_LOG_WARN(message)      LOG_WARN("CommonSettingService",message,LOGGER_NAME)
#define SERVICE_LOG_ERROR(message)     LOG_ERROR("CommonSettingService",message,LOGGER_NAME)
#define SERVICE_LOG_FATAL(message)     LOG_FATAL("CommonSettingService",message,LOGGER_NAME)