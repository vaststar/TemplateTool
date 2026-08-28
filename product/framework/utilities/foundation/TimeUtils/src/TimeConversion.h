#pragma once

#include <chrono>
#include <charconv>
#include <cstdint>
#include <limits>
#include <string_view>
#include <system_error>

#include <ucf/utilities/TimeUtils/TimeResult.h>

namespace ucf::utilities::detail {

inline constexpr int64_t kMillisecondsPerSecond = 1000;
inline constexpr int64_t kMillisecondsPerMinute = 60 * kMillisecondsPerSecond;
inline constexpr int64_t kMillisecondsPerHour = 60 * kMillisecondsPerMinute;
inline constexpr int64_t kMillisecondsPerDay = 24 * kMillisecondsPerHour;

struct CalendarFields final
{
    int year{0};
    unsigned month{0};
    unsigned day{0};
    unsigned hour{0};
    unsigned minute{0};
    unsigned second{0};
    unsigned millisecond{0};
};

inline bool checkedAdd(int64_t lhs, int64_t rhs, int64_t& result) noexcept
{
    if ((rhs > 0 && lhs > std::numeric_limits<int64_t>::max() - rhs)
        || (rhs < 0 && lhs < std::numeric_limits<int64_t>::min() - rhs))
    {
        return false;
    }
    result = lhs + rhs;
    return true;
}

inline bool checkedSubtract(int64_t lhs, int64_t rhs, int64_t& result) noexcept
{
    if ((rhs > 0 && lhs < std::numeric_limits<int64_t>::min() + rhs)
        || (rhs < 0 && lhs > std::numeric_limits<int64_t>::max() + rhs))
    {
        return false;
    }
    result = lhs - rhs;
    return true;
}

inline bool checkedMultiply(int64_t lhs, int64_t rhs, int64_t& result) noexcept
{
    if (lhs == 0 || rhs == 0)
    {
        result = 0;
        return true;
    }
    if (lhs == -1)
    {
        if (rhs == std::numeric_limits<int64_t>::min())
        {
            return false;
        }
        result = -rhs;
        return true;
    }
    if (rhs == -1)
    {
        if (lhs == std::numeric_limits<int64_t>::min())
        {
            return false;
        }
        result = -lhs;
        return true;
    }
    if ((lhs > 0 && rhs > 0 && lhs > std::numeric_limits<int64_t>::max() / rhs)
        || (lhs > 0 && rhs < 0 && rhs < std::numeric_limits<int64_t>::min() / lhs)
        || (lhs < 0 && rhs > 0 && lhs < std::numeric_limits<int64_t>::min() / rhs)
        || (lhs < 0 && rhs < 0 && lhs < std::numeric_limits<int64_t>::max() / rhs))
    {
        return false;
    }
    result = lhs * rhs;
    return true;
}

inline bool parseUnsigned(std::string_view text, unsigned& value) noexcept
{
    if (text.empty())
    {
        return false;
    }
    const char* const end = text.data() + text.size();
    const auto [position, error] = std::from_chars(text.data(), end, value);
    return error == std::errc{} && position == end;
}

inline bool parseYear(std::string_view text, int& value) noexcept
{
    if (text.empty())
    {
        return false;
    }
    const char* const end = text.data() + text.size();
    const auto [position, error] = std::from_chars(text.data(), end, value);
    return error == std::errc{} && position == end;
}

inline int64_t floorDivide(int64_t value, int64_t divisor) noexcept
{
    const int64_t quotient = value / divisor;
    const int64_t remainder = value % divisor;
    return remainder < 0 ? quotient - 1 : quotient;
}

inline TimeResult<int64_t> calendarToUnixMilliseconds(
    const CalendarFields& fields,
    int32_t offsetSeconds = 0)
{
    if (fields.year < static_cast<int>(std::chrono::year::min())
        || fields.year > static_cast<int>(std::chrono::year::max()))
    {
        return TimeResult<int64_t>::failure(
            TimeErrorCode::OutOfRange,
            "calendar year is outside std::chrono::year range");
    }
    if (fields.month < 1 || fields.month > 12 || fields.day < 1 || fields.day > 31)
    {
        return TimeResult<int64_t>::failure(
            TimeErrorCode::InvalidDate,
            "calendar month or day is outside its valid range");
    }
    const std::chrono::year_month_day date{
        std::chrono::year{fields.year},
        std::chrono::month{fields.month},
        std::chrono::day{fields.day}};
    if (!date.ok())
    {
        return TimeResult<int64_t>::failure(
            TimeErrorCode::InvalidDate,
            "invalid proleptic-Gregorian date");
    }
    if (fields.hour >= 24 || fields.minute >= 60 || fields.second >= 60
        || fields.millisecond >= 1000)
    {
        return TimeResult<int64_t>::failure(
            TimeErrorCode::InvalidTime,
            "time-of-day is outside its valid range");
    }

    const int64_t dayCount = std::chrono::sys_days{date}.time_since_epoch().count();
    int64_t result = 0;
    if (!checkedMultiply(dayCount, kMillisecondsPerDay, result)
        || !checkedAdd(result, static_cast<int64_t>(fields.hour) * kMillisecondsPerHour, result)
        || !checkedAdd(result, static_cast<int64_t>(fields.minute) * kMillisecondsPerMinute, result)
        || !checkedAdd(result, static_cast<int64_t>(fields.second) * kMillisecondsPerSecond, result)
        || !checkedAdd(result, static_cast<int64_t>(fields.millisecond), result)
        || !checkedSubtract(result, static_cast<int64_t>(offsetSeconds) * kMillisecondsPerSecond, result))
    {
        return TimeResult<int64_t>::failure(
            TimeErrorCode::OutOfRange,
            "calendar value cannot be represented as Unix milliseconds");
    }
    return TimeResult<int64_t>::success(result);
}

inline TimeResult<CalendarFields> unixMillisecondsToCalendar(
    int64_t unixMilliseconds,
    int32_t offsetSeconds = 0)
{
    int64_t localMilliseconds = 0;
    if (!checkedAdd(
            unixMilliseconds,
            static_cast<int64_t>(offsetSeconds) * kMillisecondsPerSecond,
            localMilliseconds))
    {
        return TimeResult<CalendarFields>::failure(
            TimeErrorCode::OutOfRange,
            "timezone projection exceeds the Instant range");
    }

    constexpr auto minimumDay = std::chrono::sys_days{
        std::chrono::year::min() / std::chrono::January / 1};
    constexpr auto maximumDay = std::chrono::sys_days{
        std::chrono::year::max() / std::chrono::December / 31};
    const int64_t dayCount = floorDivide(localMilliseconds, kMillisecondsPerDay);
    if (dayCount < minimumDay.time_since_epoch().count()
        || dayCount > maximumDay.time_since_epoch().count())
    {
        return TimeResult<CalendarFields>::failure(
            TimeErrorCode::OutOfRange,
            "calendar year is outside std::chrono::year range");
    }

    const auto dayPoint = std::chrono::sys_days{
        std::chrono::days{static_cast<std::chrono::days::rep>(dayCount)}};
    const std::chrono::year_month_day date{dayPoint};
    int64_t millisecondsOfDay = localMilliseconds % kMillisecondsPerDay;
    if (millisecondsOfDay < 0)
    {
        millisecondsOfDay += kMillisecondsPerDay;
    }
    CalendarFields fields;
    fields.year = static_cast<int>(date.year());
    fields.month = static_cast<unsigned>(date.month());
    fields.day = static_cast<unsigned>(date.day());
    fields.hour = static_cast<unsigned>(millisecondsOfDay / kMillisecondsPerHour);
    fields.minute = static_cast<unsigned>(
        (millisecondsOfDay % kMillisecondsPerHour) / kMillisecondsPerMinute);
    fields.second = static_cast<unsigned>(
        (millisecondsOfDay % kMillisecondsPerMinute) / kMillisecondsPerSecond);
    fields.millisecond = static_cast<unsigned>(millisecondsOfDay % kMillisecondsPerSecond);
    return TimeResult<CalendarFields>::success(fields);
}

} // namespace ucf::utilities::detail
