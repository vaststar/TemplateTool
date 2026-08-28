#include <ucf/utilities/TimeUtils/Instant.h>

#include "TimeConversion.h"

#include <cstdio>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace ucf::utilities {

namespace {

TimeResult<Instant> invalidRfc3339(std::string diagnostic)
{
    return TimeResult<Instant>::failure(
        TimeErrorCode::InvalidFormat,
        std::move(diagnostic));
}

} // namespace

Instant::Instant(TimePoint timePoint) noexcept
    : mTimePoint{timePoint}
{
}

Instant Instant::fromUnixMilliseconds(int64_t milliseconds) noexcept
{
    return Instant{TimePoint{std::chrono::milliseconds{milliseconds}}};
}

TimeResult<Instant> Instant::fromUnixSeconds(int64_t seconds)
{
    int64_t milliseconds = 0;
    if (!detail::checkedMultiply(seconds, detail::kMillisecondsPerSecond, milliseconds))
    {
        return TimeResult<Instant>::failure(
            TimeErrorCode::OutOfRange,
            "Unix seconds cannot be represented with millisecond precision");
    }
    return TimeResult<Instant>::success(fromUnixMilliseconds(milliseconds));
}

TimeResult<Instant> Instant::fromTimeT(std::time_t time)
{
    static_assert(std::is_integral_v<std::time_t>, "TimeUtils requires an integral time_t");
    if (!std::in_range<int64_t>(time))
    {
        return TimeResult<Instant>::failure(
            TimeErrorCode::OutOfRange,
            "time_t value exceeds the Instant range");
    }
    return fromUnixSeconds(static_cast<int64_t>(time));
}

TimeResult<Instant> Instant::parseRfc3339(std::string_view text)
{
    if (text.size() < 20)
    {
        return invalidRfc3339("RFC 3339 timestamp is too short");
    }

    std::size_t localEnd = 0;
    int32_t offsetSeconds = 0;
    if (text.back() == 'Z')
    {
        localEnd = text.size() - 1;
    }
    else
    {
        if (text.size() < 25)
        {
            return invalidRfc3339("RFC 3339 timestamp requires Z or an explicit offset");
        }
        const std::size_t offsetStart = text.size() - 6;
        if ((text[offsetStart] != '+' && text[offsetStart] != '-')
            || text[offsetStart + 3] != ':')
        {
            return invalidRfc3339("RFC 3339 offset must match +HH:MM or -HH:MM");
        }
        unsigned offsetHours = 0;
        unsigned offsetMinutes = 0;
        if (!detail::parseUnsigned(text.substr(offsetStart + 1, 2), offsetHours)
            || !detail::parseUnsigned(text.substr(offsetStart + 4, 2), offsetMinutes)
            || offsetHours > 18
            || offsetMinutes >= 60
            || (offsetHours == 18 && offsetMinutes != 0))
        {
            return invalidRfc3339("RFC 3339 offset is outside the supported +/-18:00 range");
        }
        const int32_t magnitude = static_cast<int32_t>(offsetHours * 3600 + offsetMinutes * 60);
        offsetSeconds = text[offsetStart] == '-' ? -magnitude : magnitude;
        localEnd = offsetStart;
    }

    const bool hasFraction = localEnd >= 21 && localEnd <= 23;
    if ((localEnd != 19 && !hasFraction)
        || text[4] != '-'
        || text[7] != '-'
        || text[10] != 'T'
        || text[13] != ':'
        || text[16] != ':'
        || (hasFraction && text[19] != '.'))
    {
        return invalidRfc3339("RFC 3339 timestamp has an invalid field layout");
    }

    detail::CalendarFields fields;
    if (!detail::parseYear(text.substr(0, 4), fields.year)
        || !detail::parseUnsigned(text.substr(5, 2), fields.month)
        || !detail::parseUnsigned(text.substr(8, 2), fields.day)
        || !detail::parseUnsigned(text.substr(11, 2), fields.hour)
        || !detail::parseUnsigned(text.substr(14, 2), fields.minute)
        || !detail::parseUnsigned(text.substr(17, 2), fields.second))
    {
        return invalidRfc3339("RFC 3339 timestamp contains a non-numeric field");
    }
    if (hasFraction)
    {
        const auto fraction = text.substr(20, localEnd - 20);
        if (!detail::parseUnsigned(fraction, fields.millisecond))
        {
            return invalidRfc3339("RFC 3339 fractional second is not numeric");
        }
        if (fraction.size() == 1)
        {
            fields.millisecond *= 100;
        }
        else if (fraction.size() == 2)
        {
            fields.millisecond *= 10;
        }
    }

    auto milliseconds = detail::calendarToUnixMilliseconds(fields, offsetSeconds);
    if (!milliseconds)
    {
        return TimeResult<Instant>::failure(
            milliseconds.error().code,
            milliseconds.error().diagnostic);
    }
    return TimeResult<Instant>::success(
        fromUnixMilliseconds(std::move(milliseconds).value()));
}

int64_t Instant::toUnixMilliseconds() const noexcept
{
    return mTimePoint.time_since_epoch().count();
}

int64_t Instant::toUnixSeconds() const noexcept
{
    return detail::floorDivide(toUnixMilliseconds(), detail::kMillisecondsPerSecond);
}

TimeResult<std::time_t> Instant::toTimeT() const
{
    const int64_t seconds = toUnixSeconds();
    if (!std::in_range<std::time_t>(seconds))
    {
        return TimeResult<std::time_t>::failure(
            TimeErrorCode::OutOfRange,
            "Instant seconds cannot be represented by time_t");
    }
    return TimeResult<std::time_t>::success(static_cast<std::time_t>(seconds));
}

TimeResult<std::string> Instant::toRfc3339() const
{
    auto fields = detail::unixMillisecondsToCalendar(toUnixMilliseconds());
    if (!fields)
    {
        return TimeResult<std::string>::failure(
            fields.error().code,
            fields.error().diagnostic);
    }
    const auto value = std::move(fields).value();
    if (value.year < 0 || value.year > 9999)
    {
        return TimeResult<std::string>::failure(
            TimeErrorCode::OutOfRange,
            "RFC 3339 representation requires a four-digit year");
    }

    char buffer[32]{};
    const int length = std::snprintf(
        buffer,
        sizeof(buffer),
        "%04d-%02u-%02uT%02u:%02u:%02u.%03uZ",
        value.year,
        value.month,
        value.day,
        value.hour,
        value.minute,
        value.second,
        value.millisecond);
    if (length <= 0 || static_cast<std::size_t>(length) >= sizeof(buffer))
    {
        return TimeResult<std::string>::failure(
            TimeErrorCode::OutOfRange,
            "RFC 3339 output exceeded its fixed representation");
    }
    return TimeResult<std::string>::success(
        std::string{buffer, static_cast<std::size_t>(length)});
}

Instant& Instant::operator+=(std::chrono::milliseconds duration)
{
    int64_t result = 0;
    if (!detail::checkedAdd(toUnixMilliseconds(), duration.count(), result))
    {
        throw std::overflow_error{"Instant addition overflow"};
    }
    mTimePoint = TimePoint{std::chrono::milliseconds{result}};
    return *this;
}

Instant& Instant::operator-=(std::chrono::milliseconds duration)
{
    int64_t result = 0;
    if (!detail::checkedSubtract(toUnixMilliseconds(), duration.count(), result))
    {
        throw std::overflow_error{"Instant subtraction overflow"};
    }
    mTimePoint = TimePoint{std::chrono::milliseconds{result}};
    return *this;
}

std::strong_ordering operator<=>(const Instant& lhs, const Instant& rhs) noexcept
{
    return lhs.mTimePoint <=> rhs.mTimePoint;
}

bool operator==(const Instant& lhs, const Instant& rhs) noexcept
{
    return lhs.mTimePoint == rhs.mTimePoint;
}

Instant operator+(Instant lhs, std::chrono::milliseconds rhs)
{
    lhs += rhs;
    return lhs;
}

Instant operator-(Instant lhs, std::chrono::milliseconds rhs)
{
    lhs -= rhs;
    return lhs;
}

std::chrono::milliseconds operator-(const Instant& lhs, const Instant& rhs)
{
    int64_t difference = 0;
    if (!detail::checkedSubtract(
            lhs.toUnixMilliseconds(),
            rhs.toUnixMilliseconds(),
            difference))
    {
        throw std::overflow_error{"Instant difference overflow"};
    }
    return std::chrono::milliseconds{difference};
}

} // namespace ucf::utilities
