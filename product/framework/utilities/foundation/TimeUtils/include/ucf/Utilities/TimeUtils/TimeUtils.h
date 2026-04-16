#pragma once

#include <cstdint>
#include <ctime>
#include <string>

#include <ucf/Utilities/UtilitiesCommonFile/UtilitiesExport.h>

namespace ucf::utilities {
class Utilities_EXPORT TimeUtils final
{
public:
    // ---- Current time as numeric values ----

    /// Returns current UTC time in milliseconds since Unix epoch
    static int64_t getCurrentUTCMilliseconds();

    /// Returns current UTC time in seconds since Unix epoch (std::time_t)
    static std::time_t getCurrentUTCSeconds();

    // ---- Current time as fixed-format strings ----

    /// Returns current UTC time as "YYYY-MM-DD HH:MM:SS"
    static std::string getCurrentUTCTimeString();

    /// Returns local timezone name, e.g. "Asia/Shanghai", "America/New_York"
    static std::string getLocalTimeZoneName();

    // ---- Custom format (caller provides time_t) ----
    // pattern follows std::strftime specification, e.g. "%Y-%m-%d %H:%M:%S"
    // Empty pattern returns empty string; invalid pattern falls back to "%Y-%m-%d %H:%M:%S"

    /// Format a time_t value as local time string with the given pattern
    static std::string formatLocalTime(std::time_t time, const std::string& pattern);

    /// Format a time_t value as UTC time string with the given pattern
    static std::string formatUTCTime(std::time_t time, const std::string& pattern);

    // ---- Custom format (auto-captures current time) ----

    /// Format current local time with the given pattern
    static std::string formatCurrentLocalTime(const std::string& pattern);

    /// Format current UTC time with the given pattern
    static std::string formatCurrentUTCTime(const std::string& pattern);

    // ---- Duration formatting ----

    /// Format seconds to "HH:MM:SS", e.g. 3661 -> "01:01:01"
    static std::string formatSecondsToHMS(int totalSeconds);

    /// Format seconds to "MM:SS", e.g. 125 -> "02:05"
    static std::string formatSecondsToMS(int totalSeconds);

public:
    TimeUtils() = delete;
    TimeUtils(const TimeUtils& rhs) = delete;
    TimeUtils& operator=(const TimeUtils& rhs) = delete;
    TimeUtils(TimeUtils&& rhs) = delete;
    TimeUtils& operator=(TimeUtils&& rhs) = delete;
    ~TimeUtils() = delete;
};
}
