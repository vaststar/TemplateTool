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

    // LogLevel::Off disables this output.
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

    // LogLevel::Off disables this output; no file operations are performed.
    LogLevel minimumLevel{LogLevel::Info};
    // The caller must ensure that this active path has only one Runtime/process writer.
    std::filesystem::path directory;
    std::filesystem::path baseName{"application"};
    std::uint64_t maxFileBytes{20U * 1024U * 1024U};
    // Zero disables archive retention cleanup.
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
