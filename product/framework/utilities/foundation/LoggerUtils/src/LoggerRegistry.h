#pragma once

#include <ucf/utilities/LoggerUtils/Config.h>

#include <source_location>
#include <string_view>

namespace ucf::utilities::detail::logger_registry {

void initialize(LoggingConfig config);
void write(std::string_view loggerName, std::string_view category, LogLevel level,
           std::string_view message, std::source_location location) noexcept;
void shutdown() noexcept;

} // namespace ucf::utilities::detail::logger_registry
