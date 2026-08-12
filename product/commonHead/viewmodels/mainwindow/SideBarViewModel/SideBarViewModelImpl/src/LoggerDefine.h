#pragma once

#include <ucf/utilities/LoggerUtils/LoggerExport.h>

static inline const char* SideBarViewModelLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* SideBarViewModelLoggerTag = "SideBarViewModel";

#define SIDE_BAR_VIEW_MODEL_LOG_DEBUG(message) UCF_LOG_DEBUG(SideBarViewModelLoggerTag, message, SideBarViewModelLoggerName)
#define SIDE_BAR_VIEW_MODEL_LOG_INFO(message) UCF_LOG_INFO(SideBarViewModelLoggerTag, message, SideBarViewModelLoggerName)
#define SIDE_BAR_VIEW_MODEL_LOG_WARN(message) UCF_LOG_WARN(SideBarViewModelLoggerTag, message, SideBarViewModelLoggerName)
#define SIDE_BAR_VIEW_MODEL_LOG_ERROR(message) UCF_LOG_ERROR(SideBarViewModelLoggerTag, message, SideBarViewModelLoggerName)
#define SIDE_BAR_VIEW_MODEL_LOG_FATAL(message) UCF_LOG_FATAL(SideBarViewModelLoggerTag, message, SideBarViewModelLoggerName)
