#pragma once

#include "MasterLog/LogExport.h"

#define WIN_LOG_DEBUG(message)     LOG_DEBUG("Windows",message,"APP")
#define WIN_LOG_INFO(message)      LOG_INFO("Windows",message,"APP")
#define WIN_LOG_WARN(message)      LOG_WARN("Windows",message,"APP")
#define WIN_LOG_ERROR(message)     LOG_ERROR("Windows",message,"APP")
#define WIN_LOG_FATAL(message)     LOG_FATAL("Windows",message,"APP")