#pragma once

#include <source_location>
#include <sstream>
#include <string_view>

#include <ucf/utilities/LoggerUtils/Config.h>
#include <ucf/utilities/LoggerUtils/LoggerUtilsExport.h>

namespace ucf::utilities {

inline constexpr const char* kAppLoggerName = "APP";

LOGGER_UTILS_API void initializeLogging(LoggingConfig config);
// Terminal and idempotent. Logging cannot be initialized again after this call.
LOGGER_UTILS_API void shutdownLogging() noexcept;

namespace detail {

LOGGER_UTILS_API void writeLog(std::string_view loggerName, std::string_view category, LogLevel level,
                               std::string_view message, std::source_location location) noexcept;

} // namespace detail

} // namespace ucf::utilities

#define UCF_LOG_DEBUG(tag, message, loggerName)                                      \
    do                                                                              \
    {                                                                               \
        ::std::ostringstream ucfLoggerUtilsStream;                                  \
        ucfLoggerUtilsStream << message;                                             \
        ::ucf::utilities::detail::writeLog((loggerName), (tag),                     \
                                           ::ucf::utilities::LogLevel::Debug,        \
                                           ucfLoggerUtilsStream.str(),               \
                                           ::std::source_location::current());       \
    } while (false)

#define UCF_LOG_INFO(tag, message, loggerName)                                       \
    do                                                                              \
    {                                                                               \
        ::std::ostringstream ucfLoggerUtilsStream;                                  \
        ucfLoggerUtilsStream << message;                                             \
        ::ucf::utilities::detail::writeLog((loggerName), (tag),                     \
                                           ::ucf::utilities::LogLevel::Info,         \
                                           ucfLoggerUtilsStream.str(),               \
                                           ::std::source_location::current());       \
    } while (false)

#define UCF_LOG_WARN(tag, message, loggerName)                                       \
    do                                                                              \
    {                                                                               \
        ::std::ostringstream ucfLoggerUtilsStream;                                  \
        ucfLoggerUtilsStream << message;                                             \
        ::ucf::utilities::detail::writeLog((loggerName), (tag),                     \
                                           ::ucf::utilities::LogLevel::Warn,         \
                                           ucfLoggerUtilsStream.str(),               \
                                           ::std::source_location::current());       \
    } while (false)

#define UCF_LOG_ERROR(tag, message, loggerName)                                      \
    do                                                                              \
    {                                                                               \
        ::std::ostringstream ucfLoggerUtilsStream;                                  \
        ucfLoggerUtilsStream << message;                                             \
        ::ucf::utilities::detail::writeLog((loggerName), (tag),                     \
                                           ::ucf::utilities::LogLevel::Error,        \
                                           ucfLoggerUtilsStream.str(),               \
                                           ::std::source_location::current());       \
    } while (false)

#define UCF_LOG_FATAL(tag, message, loggerName)                                      \
    do                                                                              \
    {                                                                               \
        ::std::ostringstream ucfLoggerUtilsStream;                                  \
        ucfLoggerUtilsStream << message;                                             \
        ::ucf::utilities::detail::writeLog((loggerName), (tag),                     \
                                           ::ucf::utilities::LogLevel::Fatal,        \
                                           ucfLoggerUtilsStream.str(),               \
                                           ::std::source_location::current());       \
    } while (false)
