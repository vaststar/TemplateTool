#pragma once

#include "MasterLog/LogExport.h"

static inline const char * UIComponentLoggerName = "APP";
#define UICOM_LOG_DEBUG(message)     LOG_DEBUG("UIComponent",message,UIComponentLoggerName)
#define UICOM_LOG_INFO(message)      LOG_INFO("UIComponent",message,UIComponentLoggerName)
#define UICOM_LOG_WARN(message)      LOG_WARN("UIComponent",message,UIComponentLoggerName)
#define UICOM_LOG_ERROR(message)     LOG_ERROR("UIComponent",message,UIComponentLoggerName)
#define UICOM_LOG_FATAL(message)     LOG_FATAL("UIComponent",message,UIComponentLoggerName)