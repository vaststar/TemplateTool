#pragma once

#include <MasterLog/LogExport.h>

static inline const char * MainUILoggerName = "APP";
#define MAINUI_LOG_DEBUG(message)     LOG_DEBUG("CoreFramework",message,MainUILoggerName)
#define MAINUI_LOG_INFO(message)      LOG_INFO("CoreFramework",message,MainUILoggerName)
#define MAINUI_LOG_WARN(message)      LOG_WARN("CoreFramework",message,MainUILoggerName)
#define MAINUI_LOG_ERROR(message)     LOG_ERROR("CoreFramework",message,MainUILoggerName)
#define MAINUI_LOG_FATAL(message)     LOG_FATAL("CoreFramework",message,MainUILoggerName)