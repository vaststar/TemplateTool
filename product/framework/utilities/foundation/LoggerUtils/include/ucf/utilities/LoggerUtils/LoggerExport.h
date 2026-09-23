#pragma once

#include <source_location>
#include <sstream>
#include <string_view>

#include <ucf/utilities/LoggerUtils/Config.h>
#include <ucf/utilities/LoggerUtils/LoggerUtilsExport.h>

namespace ucf::utilities {

inline constexpr const char* kAppLoggerName = "APP";

LOGGER_UTILS_API void initializeLogging(LoggingConfig config);
// Waits for records already accepted by SableLog and flushes every output.
// Safe before initialization, during shutdown and after shutdown.
LOGGER_UTILS_API void flushLogging() noexcept;
// Terminal and idempotent. Logging cannot be initialized again after this call.
LOGGER_UTILS_API void shutdownLogging() noexcept;

namespace detail {

LOGGER_UTILS_API void writeLog(std::string_view loggerName, std::string_view category, LogLevel level,
                               std::string_view message, std::source_location location) noexcept;

} // namespace detail

} // namespace ucf::utilities

#define UCF_LOG_DETAIL_WRITE(level, tag, message, loggerName, sourceLocation)          \
    do                                                                                \
    {                                                                                 \
        try                                                                           \
        {                                                                             \
            ::std::ostringstream ucfLoggerUtilsStream;                                \
            ucfLoggerUtilsStream << message;                                          \
            ::ucf::utilities::detail::writeLog((loggerName), (tag), (level),          \
                                               ucfLoggerUtilsStream.str(),            \
                                               (sourceLocation));                     \
        }                                                                             \
        catch (...)                                                                   \
        {                                                                             \
            /* Logging failures must not affect business execution. */                \
        }                                                                             \
    } while (false)

#define UCF_LOG_DEBUG(tag, message, loggerName)                                        \
    UCF_LOG_DETAIL_WRITE(::ucf::utilities::LogLevel::Debug, tag, message, loggerName, \
                         ::std::source_location::current())

#define UCF_LOG_INFO(tag, message, loggerName)                                        \
    UCF_LOG_DETAIL_WRITE(::ucf::utilities::LogLevel::Info, tag, message, loggerName, \
                         ::std::source_location::current())

#define UCF_LOG_WARN(tag, message, loggerName)                                        \
    UCF_LOG_DETAIL_WRITE(::ucf::utilities::LogLevel::Warn, tag, message, loggerName, \
                         ::std::source_location::current())

#define UCF_LOG_ERROR(tag, message, loggerName)                                        \
    UCF_LOG_DETAIL_WRITE(::ucf::utilities::LogLevel::Error, tag, message, loggerName, \
                         ::std::source_location::current())

#define UCF_LOG_FATAL(tag, message, loggerName)                                        \
    UCF_LOG_DETAIL_WRITE(::ucf::utilities::LogLevel::Fatal, tag, message, loggerName, \
                         ::std::source_location::current())
