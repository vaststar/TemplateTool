#pragma once

#include <MasterLog/LogExport.h>

static inline constexpr auto LOGGER_NAME = "APP";
#define SERVICE_LOG_DEBUG(message)     LOG_DEBUG("NetworkService",message,LOGGER_NAME)
#define SERVICE_LOG_INFO(message)      LOG_INFO("NetworkService",message,LOGGER_NAME)
#define SERVICE_LOG_WARN(message)      LOG_WARN("NetworkService",message,LOGGER_NAME)
#define SERVICE_LOG_ERROR(message)     LOG_ERROR("NetworkService",message,LOGGER_NAME)
#define SERVICE_LOG_FATAL(message)     LOG_FATAL("NetworkService",message,LOGGER_NAME)