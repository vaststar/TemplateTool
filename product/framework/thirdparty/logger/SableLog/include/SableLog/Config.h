#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace sablelog {

enum class Level : std::uint8_t
{
    Trace,
    Debug,
    Info,
    Warn,
    Error,
    Fatal,
    Off,
};

struct ConsoleConfig final
{
    enum class ColorMode : std::uint8_t
    {
        // Use colors only for an interactive terminal that supports them.
        // Redirected output is plain text.
        Automatic,
        // Always emit ANSI color sequences, including redirected output.
        Always,
        // Never emit color sequences.
        Never,
    };

    Level minimumLevel{Level::Info};
    ColorMode colorMode{ColorMode::Automatic};
};

struct FileConfig final
{
    enum class CalendarRotation : std::uint8_t
    {
        None,
        Daily,
        Monthly,
    };

    Level minimumLevel{Level::Info};
    std::filesystem::path directory;
    // A filename stem, not a path. The active file is <baseName>.log; rotated files are
    // <baseName>.YYYYMMDD.<sequence>.log, using the last successful UTC write date.
    std::filesystem::path baseName{"application"};
    std::uint64_t maxFileBytes{20U * 1024U * 1024U};
    std::uint32_t retentionDays{180U};
    CalendarRotation calendarRotation{CalendarRotation::Daily};
};

struct LoggerConfig final
{
    std::string loggerName;
    std::optional<ConsoleConfig> console;
    std::vector<FileConfig> files;
};

struct RuntimeConfig final
{
    std::vector<LoggerConfig> loggers;
};

} // namespace sablelog
