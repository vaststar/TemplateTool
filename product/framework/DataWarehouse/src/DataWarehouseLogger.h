#pragma once

#include <MasterLog/LogExport.h>

//for coreframework
static inline const char * FrameworkLoggerName = "APP";
#define DATAWAREHOUSE_LOG_DEBUG(message)     LOG_DEBUG("DataWarehouse",message,FrameworkLoggerName)
#define DATAWAREHOUSE_LOG_INFO(message)      LOG_INFO("DataWarehouse",message,FrameworkLoggerName)
#define DATAWAREHOUSE_LOG_WARN(message)      LOG_WARN("DataWarehouse",message,FrameworkLoggerName)
#define DATAWAREHOUSE_LOG_ERROR(message)     LOG_ERROR("DataWarehouse",message,FrameworkLoggerName)
#define DATAWAREHOUSE_LOG_FATAL(message)     LOG_FATAL("DataWarehouse",message,FrameworkLoggerName)