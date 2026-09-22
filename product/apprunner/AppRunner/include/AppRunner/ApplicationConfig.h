#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace AppRunner
{
    struct StartupConfig final
    {
        std::vector<std::string> commandLineArguments;
    };

    struct AppLogConfig final
    {
        enum class Level : std::uint8_t
        {
            Debug,
            Info,
            Warn,
            Error,
            Fatal,
            Off,
        };

        enum class CalendarRotation : std::uint8_t
        {
            None,
            Daily,
            Monthly,
        };

        std::filesystem::path directory;
        std::filesystem::path baseName{"AppLog"};
        Level minimumLevel{Level::Debug};
        std::uint64_t maxFileBytes{50ULL * 1024ULL * 1024ULL};
        std::uint32_t retentionDays{180U};
        CalendarRotation calendarRotation{CalendarRotation::Daily};
    };

    struct ApplicationConfig final
    {
        StartupConfig startupConfig;
        AppLogConfig appLogConfig;
    };
}
