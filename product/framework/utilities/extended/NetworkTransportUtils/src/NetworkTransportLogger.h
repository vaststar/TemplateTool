#pragma once

#include <ucf/utilities/LoggerUtils/LoggerExport.h>

static inline const char* NetworkTransportLoggerName = ucf::utilities::kAppLoggerName;

// Logging is diagnostic and must never be able to terminate an I/O handler.
// The underlying logger builds an ostringstream and can therefore throw on an
// allocation failure, even when the surrounding networking path is noexcept.
#define NETWORK_TRANSPORT_LOG_IMPL(logMacro, tag, message) \
    do \
    { \
        try \
        { \
            logMacro(tag, message, NetworkTransportLoggerName); \
        } \
        catch (...) \
        { \
        } \
    } while (false)

#define NETWORK_TRANSPORT_LOG_DEBUG(tag, message) NETWORK_TRANSPORT_LOG_IMPL(UCF_LOG_DEBUG, tag, message)
#define NETWORK_TRANSPORT_LOG_INFO(tag, message) NETWORK_TRANSPORT_LOG_IMPL(UCF_LOG_INFO, tag, message)
#define NETWORK_TRANSPORT_LOG_WARN(tag, message) NETWORK_TRANSPORT_LOG_IMPL(UCF_LOG_WARN, tag, message)
#define NETWORK_TRANSPORT_LOG_ERROR(tag, message) NETWORK_TRANSPORT_LOG_IMPL(UCF_LOG_ERROR, tag, message)
#define NETWORK_TRANSPORT_LOG_FATAL(tag, message) NETWORK_TRANSPORT_LOG_IMPL(UCF_LOG_FATAL, tag, message)
