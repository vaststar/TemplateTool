#pragma once

#include <chrono>
#include <cstdint>
#include <ctime>
#include <string>

#include <ucf/Utilities/UtilitiesCommonFile/UtilitiesExport.h>

namespace ucf::utilities {

// Non-instantiable bag of free helpers that complement Instant /
// LocalDate / LocalDateTime. Prefer the value types for new code;
// the helpers here are kept for ergonomics at boundaries (time_t,
// int64_t ms) and for duration formatting.
class Utilities_EXPORT TimeUtils final
{
public:
    // ---- Current time as numeric values ----
    static int64_t getCurrentUTCMilliseconds();
    static std::time_t getCurrentUTCSeconds();

    // ---- Current time as fixed-format strings ----
    // "YYYY-MM-DD HH:MM:SS".
    static std::string getCurrentUTCTimeString();
    // IANA name when available, otherwise abbreviation, otherwise "UTC".
    static std::string getLocalTimeZoneName();

    // ---- Custom format (caller provides time_t) ----
    // Pattern follows std::strftime. Empty pattern returns empty string;
    // invalid pattern falls back to "%Y-%m-%d %H:%M:%S".
    static std::string formatLocalTime(std::time_t time, const std::string& pattern);
    static std::string formatUTCTime(std::time_t time, const std::string& pattern);

    // ---- Custom format (auto-captures current time) ----
    static std::string formatCurrentLocalTime(const std::string& pattern);
    static std::string formatCurrentUTCTime(const std::string& pattern);

    // ---- Duration formatting ----
    // "HH:MM:SS", e.g. 3661 -> "01:01:01". Negative clamps to zero.
    static std::string formatSecondsToHMS(int totalSeconds);
    // "MM:SS", e.g. 125 -> "02:05". Negative clamps to zero.
    static std::string formatSecondsToMS(int totalSeconds);

    // "HH:MM:SS.fff", millisecond precision. Negative clamps to zero.
    static std::string formatDuration(std::chrono::milliseconds duration);
    // Human-readable, e.g. "1h 2m 3s", "500 ms". Negative clamps to zero.
    static std::string formatDurationHuman(std::chrono::milliseconds duration);

public:
    TimeUtils() = delete;
    TimeUtils(const TimeUtils& rhs) = delete;
    TimeUtils& operator=(const TimeUtils& rhs) = delete;
    TimeUtils(TimeUtils&& rhs) = delete;
    TimeUtils& operator=(TimeUtils&& rhs) = delete;
    ~TimeUtils() = delete;
};

} // namespace ucf::utilities
