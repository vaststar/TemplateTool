#pragma once

#include <MasterLog/LogExport.h>

static inline const char * UIFabricationLoggerName = "APP";
#define UIFabrication_LOG_DEBUG(message)     LOG_DEBUG("UIFabrication",message,UIFabricationLoggerName)
#define UIFabrication_LOG_INFO(message)      LOG_INFO("UIFabrication",message,UIFabricationLoggerName)
#define UIFabrication_LOG_WARN(message)      LOG_WARN("UIFabrication",message,UIFabricationLoggerName)
#define UIFabrication_LOG_ERROR(message)     LOG_ERROR("UIFabrication",message,UIFabricationLoggerName)
#define UIFabrication_LOG_FATAL(message)     LOG_FATAL("UIFabrication",message,UIFabricationLoggerName)