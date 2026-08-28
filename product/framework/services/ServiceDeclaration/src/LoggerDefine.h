#pragma once

#include <ucf/utilities/LoggerUtils/LoggerExport.h>

static inline const char* ServiceDeclarationLoggerName =
    ucf::utilities::kAppLoggerName;
static inline const char* ServiceDeclarationLoggerTag =
    "ServiceDeclaration";

#define SERVICE_DECLARATION_LOG_DEBUG(message) \
    UCF_LOG_DEBUG(                              \
        ServiceDeclarationLoggerTag,            \
        message,                                \
        ServiceDeclarationLoggerName)

#define SERVICE_DECLARATION_LOG_INFO(message) \
    UCF_LOG_INFO(                              \
        ServiceDeclarationLoggerTag,           \
        message,                               \
        ServiceDeclarationLoggerName)

#define SERVICE_DECLARATION_LOG_WARN(message) \
    UCF_LOG_WARN(                              \
        ServiceDeclarationLoggerTag,           \
        message,                               \
        ServiceDeclarationLoggerName)

#define SERVICE_DECLARATION_LOG_ERROR(message) \
    UCF_LOG_ERROR(                              \
        ServiceDeclarationLoggerTag,            \
        message,                                \
        ServiceDeclarationLoggerName)

#define SERVICE_DECLARATION_LOG_FATAL(message) \
    UCF_LOG_FATAL(                              \
        ServiceDeclarationLoggerTag,            \
        message,                                \
        ServiceDeclarationLoggerName)
