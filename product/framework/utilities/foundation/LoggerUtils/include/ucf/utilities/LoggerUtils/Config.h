#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace ucf::utilities {

enum class LogLevel : std::uint8_t
{
    Debug,
    Info,
    Warn,
    Error,
    Fatal,
    Off,
};

struct ConsoleOutputConfig final
{
    enum class ColorMode : std::uint8_t
    {
        Automatic,
        Always,
        Never,
    };

    LogLevel minimumLevel{LogLevel::Info};
    ColorMode colorMode{ColorMode::Automatic};
};

struct FileOutputConfig final
{
    enum class CalendarRotation : std::uint8_t
    {
        None,
        Daily,
        Monthly,
    };

    LogLevel minimumLevel{LogLevel::Info};
    std::filesystem::path directory;
    std::filesystem::path baseName{"application"};
    std::uint64_t maxFileBytes{20U * 1024U * 1024U};
    std::uint32_t retentionDays{180U};
    CalendarRotation calendarRotation{CalendarRotation::Daily};
};

struct LoggerConfig final
{
    std::string loggerName;
    std::optional<ConsoleOutputConfig> consoleOutput;
    std::vector<FileOutputConfig> fileOutputs;
};

struct LoggingConfig final
{
    std::vector<LoggerConfig> loggers;
};

} // namespace ucf::utilities
