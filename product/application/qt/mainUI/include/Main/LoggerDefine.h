#pragma once

#include <MasterLog/LogExport.h>

static inline const char * MainUILoggerName = "APP";
#define MAINUI_LOG_DEBUG(message)     LOG_DEBUG("MainUI",message,MainUILoggerName)
#define MAINUI_LOG_INFO(message)      LOG_INFO("MainUI",message,MainUILoggerName)
#define MAINUI_LOG_WARN(message)      LOG_WARN("MainUI",message,MainUILoggerName)
#define MAINUI_LOG_ERROR(message)     LOG_ERROR("MainUI",message,MainUILoggerName)
#define MAINUI_LOG_FATAL(message)     LOG_FATAL("MainUI",message,MainUILoggerName)