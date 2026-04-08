#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* UIComponentLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* UIComponentLoggerTag = "UIComposite";

#define UICOM_LOG_DEBUG(message)     UCF_LOG_DEBUG(UIComponentLoggerTag, message, UIComponentLoggerName)
#define UICOM_LOG_INFO(message)      UCF_LOG_INFO(UIComponentLoggerTag, message, UIComponentLoggerName)
#define UICOM_LOG_WARN(message)      UCF_LOG_WARN(UIComponentLoggerTag, message, UIComponentLoggerName)
