#pragma once

#include <ucf/utilities/LoggerUtils/LoggerExport.h>

static inline const char* MediaCameraViewModelLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* MediaCameraViewModelLoggerTag = "MediaCameraViewModel";

#define MEDIA_CAMERA_VIEW_MODEL_LOG_DEBUG(message) UCF_LOG_DEBUG(MediaCameraViewModelLoggerTag, message, MediaCameraViewModelLoggerName)
#define MEDIA_CAMERA_VIEW_MODEL_LOG_INFO(message) UCF_LOG_INFO(MediaCameraViewModelLoggerTag, message, MediaCameraViewModelLoggerName)
#define MEDIA_CAMERA_VIEW_MODEL_LOG_WARN(message) UCF_LOG_WARN(MediaCameraViewModelLoggerTag, message, MediaCameraViewModelLoggerName)
#define MEDIA_CAMERA_VIEW_MODEL_LOG_ERROR(message) UCF_LOG_ERROR(MediaCameraViewModelLoggerTag, message, MediaCameraViewModelLoggerName)
#define MEDIA_CAMERA_VIEW_MODEL_LOG_FATAL(message) UCF_LOG_FATAL(MediaCameraViewModelLoggerTag, message, MediaCameraViewModelLoggerName)
