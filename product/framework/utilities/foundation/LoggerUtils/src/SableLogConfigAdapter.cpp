#include "SableLogConfigAdapter.h"

#include <stdexcept>
#include <utility>

namespace ucf::utilities::detail {
namespace {

[[nodiscard]] sablelog::Level toSableLogLevel(LogLevel level)
{
    switch (level)
    {
    case LogLevel::Debug:
        return sablelog::Level::Debug;
    case LogLevel::Info:
        return sablelog::Level::Info;
    case LogLevel::Warn:
        return sablelog::Level::Warn;
    case LogLevel::Error:
        return sablelog::Level::Error;
    case LogLevel::Fatal:
        return sablelog::Level::Fatal;
    case LogLevel::Off:
        return sablelog::Level::Off;
    }

    throw std::invalid_argument{"LoggerUtils log level is invalid"};
}

[[nodiscard]] sablelog::ConsoleConfig::ColorMode toSableLogColorMode(
    ConsoleOutputConfig::ColorMode colorMode)
{
    switch (colorMode)
    {
    case ConsoleOutputConfig::ColorMode::Automatic:
        return sablelog::ConsoleConfig::ColorMode::Automatic;
    case ConsoleOutputConfig::ColorMode::Always:
        return sablelog::ConsoleConfig::ColorMode::Always;
    case ConsoleOutputConfig::ColorMode::Never:
        return sablelog::ConsoleConfig::ColorMode::Never;
    }

    throw std::invalid_argument{"LoggerUtils console color mode is invalid"};
}

[[nodiscard]] sablelog::FileConfig::CalendarRotation toSableLogCalendarRotation(
    FileOutputConfig::CalendarRotation calendarRotation)
{
    switch (calendarRotation)
    {
    case FileOutputConfig::CalendarRotation::None:
        return sablelog::FileConfig::CalendarRotation::None;
    case FileOutputConfig::CalendarRotation::Daily:
        return sablelog::FileConfig::CalendarRotation::Daily;
    case FileOutputConfig::CalendarRotation::Monthly:
        return sablelog::FileConfig::CalendarRotation::Monthly;
    }

    throw std::invalid_argument{"LoggerUtils calendar rotation is invalid"};
}

[[nodiscard]] sablelog::ConsoleConfig toSableLogConsoleConfig(ConsoleOutputConfig config)
{
    sablelog::ConsoleConfig result;
    result.minimumLevel = toSableLogLevel(config.minimumLevel);
    result.colorMode = toSableLogColorMode(config.colorMode);
    return result;
}

[[nodiscard]] sablelog::FileConfig toSableLogFileConfig(FileOutputConfig config)
{
    sablelog::FileConfig result;
    result.minimumLevel = toSableLogLevel(config.minimumLevel);
    result.directory = std::move(config.directory);
    result.baseName = std::move(config.baseName);
    result.maxFileBytes = config.maxFileBytes;
    result.retentionDays = config.retentionDays;
    result.calendarRotation = toSableLogCalendarRotation(config.calendarRotation);
    return result;
}

[[nodiscard]] sablelog::LoggerConfig toSableLogLoggerConfig(LoggerConfig config)
{
    sablelog::LoggerConfig result;
    result.loggerName = std::move(config.loggerName);

    if (config.consoleOutput)
    {
        result.console = toSableLogConsoleConfig(std::move(*config.consoleOutput));
    }

    result.files.reserve(config.fileOutputs.size());
    for (auto& fileConfig : config.fileOutputs)
    {
        result.files.emplace_back(toSableLogFileConfig(std::move(fileConfig)));
    }

    return result;
}

} // namespace

sablelog::RuntimeConfig toSableLogConfig(LoggingConfig config)
{
    sablelog::RuntimeConfig result;
    result.loggers.reserve(config.loggers.size());

    for (auto& loggerConfig : config.loggers)
    {
        result.loggers.emplace_back(toSableLogLoggerConfig(std::move(loggerConfig)));
    }

    return result;
}

} // namespace ucf::utilities::detail
