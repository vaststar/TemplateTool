#pragma once

#include "LogExport.h"

static inline const char * UIViewLoggerName = "APP";
#define UIVIEW_LOG_DEBUG(message)     LOG_DEBUG("UIView",message,UIViewLoggerName)
#define UIVIEW_LOG_INFO(message)      LOG_INFO("UIView",message,UIViewLoggerName)
#define UIVIEW_LOG_WARN(message)      LOG_WARN("UIView",message,UIViewLoggerName)
#define UIVIEW_LOG_ERROR(message)     LOG_ERROR("UIView",message,UIViewLoggerName)
#define UIVIEW_LOG_FATAL(message)     LOG_FATAL("UIView",message,UIViewLoggerName)