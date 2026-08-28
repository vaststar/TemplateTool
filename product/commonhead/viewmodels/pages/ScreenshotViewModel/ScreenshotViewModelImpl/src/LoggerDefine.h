#pragma once

#include <ucf/utilities/LoggerUtils/LoggerExport.h>

static inline const char* ScreenshotViewModelLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* ScreenshotViewModelLoggerTag = "ScreenshotViewModel";

#define SCREENSHOT_VIEW_MODEL_LOG_DEBUG(message) UCF_LOG_DEBUG(ScreenshotViewModelLoggerTag, message, ScreenshotViewModelLoggerName)
#define SCREENSHOT_VIEW_MODEL_LOG_INFO(message) UCF_LOG_INFO(ScreenshotViewModelLoggerTag, message, ScreenshotViewModelLoggerName)
#define SCREENSHOT_VIEW_MODEL_LOG_WARN(message) UCF_LOG_WARN(ScreenshotViewModelLoggerTag, message, ScreenshotViewModelLoggerName)
#define SCREENSHOT_VIEW_MODEL_LOG_ERROR(message) UCF_LOG_ERROR(ScreenshotViewModelLoggerTag, message, ScreenshotViewModelLoggerName)
#define SCREENSHOT_VIEW_MODEL_LOG_FATAL(message) UCF_LOG_FATAL(ScreenshotViewModelLoggerTag, message, ScreenshotViewModelLoggerName)
