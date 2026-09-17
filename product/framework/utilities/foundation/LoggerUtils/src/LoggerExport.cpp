#include <ucf/utilities/LoggerUtils/LoggerExport.h>

#include "LoggerRegistry.h"

#include <utility>

namespace ucf::utilities {

void initializeLogging(LoggingConfig config)
{
    detail::logger_registry::initialize(std::move(config));
}

void shutdownLogging() noexcept
{
    detail::logger_registry::shutdown();
}

namespace detail {

void writeLog(std::string_view loggerName, std::string_view category, LogLevel level,
              std::string_view message, std::source_location location) noexcept
{
    logger_registry::write(loggerName, category, level, message, location);
}

} // namespace detail

} // namespace ucf::utilities
