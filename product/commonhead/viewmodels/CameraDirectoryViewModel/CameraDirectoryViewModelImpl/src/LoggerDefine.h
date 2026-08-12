#pragma once

#include <ucf/utilities/LoggerUtils/LoggerExport.h>

static inline const char* CameraDirectoryViewModelLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* CameraDirectoryViewModelLoggerTag = "CameraDirectoryViewModel";

#define CAMERA_DIRECTORY_VIEW_MODEL_LOG_DEBUG(message) UCF_LOG_DEBUG(CameraDirectoryViewModelLoggerTag, message, CameraDirectoryViewModelLoggerName)
#define CAMERA_DIRECTORY_VIEW_MODEL_LOG_INFO(message) UCF_LOG_INFO(CameraDirectoryViewModelLoggerTag, message, CameraDirectoryViewModelLoggerName)
#define CAMERA_DIRECTORY_VIEW_MODEL_LOG_WARN(message) UCF_LOG_WARN(CameraDirectoryViewModelLoggerTag, message, CameraDirectoryViewModelLoggerName)
#define CAMERA_DIRECTORY_VIEW_MODEL_LOG_ERROR(message) UCF_LOG_ERROR(CameraDirectoryViewModelLoggerTag, message, CameraDirectoryViewModelLoggerName)
#define CAMERA_DIRECTORY_VIEW_MODEL_LOG_FATAL(message) UCF_LOG_FATAL(CameraDirectoryViewModelLoggerTag, message, CameraDirectoryViewModelLoggerName)
