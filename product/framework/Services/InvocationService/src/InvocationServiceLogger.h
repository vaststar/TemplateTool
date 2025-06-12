#pragma once

#include <MasterLog/LogExport.h>

static inline constexpr auto LOGGER_NAME = "APP";
#define SERVICE_LOG_DEBUG(message)     LOG_DEBUG("InvocationService",message,LOGGER_NAME)
#define SERVICE_LOG_INFO(message)      LOG_INFO("InvocationService",message,LOGGER_NAME)
#define SERVICE_LOG_WARN(message)      LOG_WARN("InvocationService",message,LOGGER_NAME)
#define SERVICE_LOG_ERROR(message)     LOG_ERROR("InvocationService",message,LOGGER_NAME)
#define SERVICE_LOG_FATAL(message)     LOG_FATAL("InvocationService",message,LOGGER_NAME)