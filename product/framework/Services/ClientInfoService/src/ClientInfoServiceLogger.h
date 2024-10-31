#pragma once

#include <MasterLog/LogExport.h>

static inline constexpr auto LOGGER_NAME = "APP";
#define SERVICE_LOG_DEBUG(message)     LOG_DEBUG("ClientInfoService",message,LOGGER_NAME)
#define SERVICE_LOG_INFO(message)      LOG_INFO("ClientInfoService",message,LOGGER_NAME)
#define SERVICE_LOG_WARN(message)      LOG_WARN("ClientInfoService",message,LOGGER_NAME)
#define SERVICE_LOG_ERROR(message)     LOG_ERROR("ClientInfoService",message,LOGGER_NAME)
#define SERVICE_LOG_FATAL(message)     LOG_FATAL("ClientInfoService",message,LOGGER_NAME)