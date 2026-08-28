#include <ucf/utilities/TimeUtils/TimeUtils.h>

#include "TimeConversion.h"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <cstdint>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <utility>

#if defined(__cpp_lib_chrono) && __cpp_lib_chrono >= 201907L
#  define UCF_TIMEUTILS_HAS_STD_TZDB 1
#else
#  define UCF_TIMEUTILS_HAS_STD_TZDB 0
#endif

namespace ucf::utilities {

namespace {

detail::CalendarFields toFields(const LocalDateTime& value) noexcept
{
    return detail::CalendarFields{
        value.date().year(),
        value.date().month(),
        value.date().day(),
        value.time().hour(),
        value.time().minute(),
        value.time().second(),
        value.time().millisecond()};
}

TimeResult<LocalDateTime> fromFields(const detail::CalendarFields& fields)
{
    auto date = LocalDate::create(fields.year, fields.month, fields.day);
    if (!date)
    {
        return TimeResult<LocalDateTime>::failure(
            date.error().code,
            date.error().diagnostic);
    }
    auto time = LocalTime::create(
        fields.hour,
        fields.minute,
        fields.second,
        fields.millisecond);
    if (!time)
    {
        return TimeResult<LocalDateTime>::failure(
            time.error().code,
            time.error().diagnostic);
    }
    return TimeResult<LocalDateTime>::success(
        LocalDateTime{std::move(date).value(), std::move(time).value()});
}

TimeResult<LocalDateTime> projectToLocalDateTime(
    Instant instant,
    int32_t offsetSeconds)
{
    auto fields = detail::unixMillisecondsToCalendar(
        instant.toUnixMilliseconds(),
        offsetSeconds);
    if (!fields)
    {
        return TimeResult<LocalDateTime>::failure(
            fields.error().code,
            fields.error().diagnostic);
    }
    return fromFields(fields.value());
}

#if UCF_TIMEUTILS_HAS_STD_TZDB

TimeResult<int64_t> localTimelineMilliseconds(const LocalDateTime& value)
{
    return detail::calendarToUnixMilliseconds(toFields(value));
}

#endif

TimeResult<std::string> invalidPattern(std::string diagnostic)
{
    return TimeResult<std::string>::failure(
        TimeErrorCode::InvalidFormat,
        std::move(diagnostic));
}

std::string twoDigits(unsigned value)
{
    std::ostringstream stream;
    stream << std::setfill('0') << std::setw(2) << value;
    return stream.str();
}

std::string threeDigits(unsigned value)
{
    std::ostringstream stream;
    stream << std::setfill('0') << std::setw(3) << value;
    return stream.str();
}

TimeResult<std::string> formatFields(
    const LocalDateTime& value,
    std::string_view pattern,
    const UtcOffset* offset,
    std::string_view abbreviation)
{
    if (pattern.empty())
    {
        return invalidPattern("time format pattern cannot be empty");
    }
    std::string output;
    output.reserve(pattern.size() + 24);
    for (std::size_t index = 0; index < pattern.size(); ++index)
    {
        if (pattern[index] != '%')
        {
            output.push_back(pattern[index]);
            continue;
        }
        if (++index == pattern.size())
        {
            return invalidPattern("time format pattern ends with an incomplete directive");
        }
        switch (pattern[index])
        {
        case '%':
            output.push_back('%');
            break;
        case 'Y':
        {
            if (value.date().year() < 0 || value.date().year() > 9999)
            {
                return TimeResult<std::string>::failure(
                    TimeErrorCode::OutOfRange,
                    "portable %Y formatting requires a four-digit year");
            }
            std::ostringstream stream;
            stream << std::setfill('0') << std::setw(4) << value.date().year();
            output += stream.str();
            break;
        }
        case 'm':
            output += twoDigits(value.date().month());
            break;
        case 'd':
            output += twoDigits(value.date().day());
            break;
        case 'H':
            output += twoDigits(value.time().hour());
            break;
        case 'M':
            output += twoDigits(value.time().minute());
            break;
        case 'S':
            output += twoDigits(value.time().second());
            break;
        case 'f':
            output += threeDigits(value.time().millisecond());
            break;
        case 'z':
            if (offset == nullptr)
            {
                return invalidPattern("%z requires a ZonedDateTime");
            }
            output += offset->toString();
            break;
        case 'Z':
            if (offset == nullptr)
            {
                return invalidPattern("%Z requires a ZonedDateTime");
            }
            output += abbreviation;
            break;
        default:
            return invalidPattern("unsupported portable time format directive");
        }
    }
    return TimeResult<std::string>::success(std::move(output));
}

enum ParseFieldBit : unsigned
{
    YearBit = 1U << 0U,
    MonthBit = 1U << 1U,
    DayBit = 1U << 2U,
    HourBit = 1U << 3U,
    MinuteBit = 1U << 4U,
    SecondBit = 1U << 5U,
    MillisecondBit = 1U << 6U
};

struct PatternParseState final
{
    detail::CalendarFields fields;
    unsigned seen{0};
};

bool parsePatternRecursive(
    std::string_view text,
    std::string_view pattern,
    std::size_t textIndex,
    std::size_t patternIndex,
    PatternParseState state,
    PatternParseState& result)
{
    if (patternIndex == pattern.size())
    {
        if (textIndex == text.size())
        {
            result = state;
            return true;
        }
        return false;
    }

    if (pattern[patternIndex] != '%')
    {
        return textIndex < text.size()
            && text[textIndex] == pattern[patternIndex]
            && parsePatternRecursive(
                text,
                pattern,
                textIndex + 1,
                patternIndex + 1,
                state,
                result);
    }
    if (patternIndex + 1 >= pattern.size())
    {
        return false;
    }

    const char directive = pattern[patternIndex + 1];
    if (directive == '%')
    {
        return textIndex < text.size()
            && text[textIndex] == '%'
            && parsePatternRecursive(
                text,
                pattern,
                textIndex + 1,
                patternIndex + 2,
                state,
                result);
    }
    if (directive == 'z' || directive == 'Z')
    {
        return false;
    }

    unsigned* target = nullptr;
    unsigned bit = 0;
    std::size_t width = 2;
    switch (directive)
    {
    case 'Y':
        bit = YearBit;
        width = 4;
        break;
    case 'm':
        bit = MonthBit;
        target = &state.fields.month;
        break;
    case 'd':
        bit = DayBit;
        target = &state.fields.day;
        break;
    case 'H':
        bit = HourBit;
        target = &state.fields.hour;
        break;
    case 'M':
        bit = MinuteBit;
        target = &state.fields.minute;
        break;
    case 'S':
        bit = SecondBit;
        target = &state.fields.second;
        break;
    case 'f':
        bit = MillisecondBit;
        break;
    default:
        return false;
    }
    if ((state.seen & bit) != 0)
    {
        return false;
    }
    state.seen |= bit;

    if (directive == 'f')
    {
        for (std::size_t fractionWidth = 1; fractionWidth <= 3; ++fractionWidth)
        {
            if (textIndex + fractionWidth > text.size())
            {
                break;
            }
            unsigned fraction = 0;
            if (!detail::parseUnsigned(text.substr(textIndex, fractionWidth), fraction))
            {
                break;
            }
            PatternParseState candidate = state;
            candidate.fields.millisecond = fractionWidth == 1
                ? fraction * 100
                : (fractionWidth == 2 ? fraction * 10 : fraction);
            if (parsePatternRecursive(
                    text,
                    pattern,
                    textIndex + fractionWidth,
                    patternIndex + 2,
                    candidate,
                    result))
            {
                return true;
            }
        }
        return false;
    }

    if (textIndex + width > text.size())
    {
        return false;
    }
    if (directive == 'Y')
    {
        if (!detail::parseYear(text.substr(textIndex, width), state.fields.year))
        {
            return false;
        }
    }
    else if (!detail::parseUnsigned(text.substr(textIndex, width), *target))
    {
        return false;
    }
    return parsePatternRecursive(
        text,
        pattern,
        textIndex + width,
        patternIndex + 2,
        state,
        result);
}

template<typename Rep>
uint64_t magnitude(Rep value) noexcept
{
    static_assert(std::is_integral_v<Rep> && std::is_signed_v<Rep>);
    using Unsigned = std::make_unsigned_t<Rep>;
    const Unsigned converted = static_cast<Unsigned>(value);
    return value < 0
        ? static_cast<uint64_t>(Unsigned{0} - converted)
        : static_cast<uint64_t>(converted);
}

std::string padAtLeastTwo(uint64_t value)
{
    std::ostringstream stream;
    stream << std::setfill('0') << std::setw(2) << value;
    return stream.str();
}

#if UCF_TIMEUTILS_HAS_STD_TZDB

TimeResult<const std::chrono::time_zone*> locateNamedTimeZone(std::string_view id)
{
    try
    {
        return TimeResult<const std::chrono::time_zone*>::success(
            std::chrono::locate_zone(std::string{id}));
    }
    catch (const std::runtime_error& error)
    {
        return TimeResult<const std::chrono::time_zone*>::failure(
            TimeErrorCode::UnknownTimeZone,
            std::string{"cannot locate named timezone: "} + error.what());
    }
}

TimeResult<int32_t> offsetSeconds(std::chrono::seconds offset)
{
    if (!std::in_range<int32_t>(offset.count()))
    {
        return TimeResult<int32_t>::failure(
            TimeErrorCode::OutOfRange,
            "timezone UTC offset exceeds int32 range");
    }
    return TimeResult<int32_t>::success(static_cast<int32_t>(offset.count()));
}

#endif

} // namespace

Instant TimeUtils::now() noexcept
{
    return SystemClock::instance().now();
}

bool TimeUtils::supportsNamedTimeZones() noexcept
{
    return UCF_TIMEUTILS_HAS_STD_TZDB != 0;
}

TimeResult<TimeZone> TimeUtils::findTimeZone(std::string_view id)
{
    if (id == "UTC")
    {
        return TimeResult<TimeZone>::success(TimeZone::utc());
    }
    if (id.empty() || id.find('/') == std::string_view::npos)
    {
        return TimeResult<TimeZone>::failure(
            TimeErrorCode::UnknownTimeZone,
            "named timezone must be an IANA region ID, not an abbreviation");
    }
#if UCF_TIMEUTILS_HAS_STD_TZDB
    auto zone = locateNamedTimeZone(id);
    if (!zone)
    {
        return TimeResult<TimeZone>::failure(
            zone.error().code,
            zone.error().diagnostic);
    }
    return TimeResult<TimeZone>::success(
        TimeZone::named(std::string{zone.value()->name()}));
#else
    return TimeResult<TimeZone>::failure(
        TimeErrorCode::TimeZoneNotSupported,
        "this standard library does not provide C++20 timezone support");
#endif
}

TimeResult<TimeZone> TimeUtils::systemTimeZone()
{
#if UCF_TIMEUTILS_HAS_STD_TZDB
    try
    {
        const auto* zone = std::chrono::current_zone();
        return TimeResult<TimeZone>::success(
            TimeZone::named(std::string{zone->name()}));
    }
    catch (const std::runtime_error& error)
    {
        return TimeResult<TimeZone>::failure(
            TimeErrorCode::TimeZoneDatabaseUnavailable,
            std::string{"cannot determine the system timezone: "} + error.what());
    }
#else
    return TimeResult<TimeZone>::failure(
        TimeErrorCode::TimeZoneNotSupported,
        "this standard library does not provide C++20 timezone support");
#endif
}

std::optional<std::string> TimeUtils::timeZoneDatabaseVersion()
{
#if UCF_TIMEUTILS_HAS_STD_TZDB
    try
    {
        return std::chrono::get_tzdb().version;
    }
    catch (const std::runtime_error&)
    {
        return std::nullopt;
    }
#else
    return std::nullopt;
#endif
}

TimeResult<ZonedDateTime> TimeUtils::toZonedDateTime(
    Instant instant,
    const TimeZone& timeZone)
{
    if (timeZone.kind() == TimeZoneKind::Utc)
    {
        auto local = projectToLocalDateTime(instant, 0);
        if (!local)
        {
            return TimeResult<ZonedDateTime>::failure(
                local.error().code,
                local.error().diagnostic);
        }
        return TimeResult<ZonedDateTime>::success(ZonedDateTime{
            instant,
            std::move(local).value(),
            timeZone,
            UtcOffset::utc(),
            "UTC",
            false});
    }
    if (timeZone.kind() == TimeZoneKind::FixedOffset)
    {
        const UtcOffset offset = *timeZone.configuredOffset();
        auto local = projectToLocalDateTime(instant, offset.totalSeconds());
        if (!local)
        {
            return TimeResult<ZonedDateTime>::failure(
                local.error().code,
                local.error().diagnostic);
        }
        return TimeResult<ZonedDateTime>::success(ZonedDateTime{
            instant,
            std::move(local).value(),
            timeZone,
            offset,
            offset.toString(),
            false});
    }

#if UCF_TIMEUTILS_HAS_STD_TZDB
    auto zone = locateNamedTimeZone(timeZone.id());
    if (!zone)
    {
        return TimeResult<ZonedDateTime>::failure(
            zone.error().code,
            zone.error().diagnostic);
    }
    try
    {
        const std::chrono::sys_time<std::chrono::milliseconds> systemTime{
            std::chrono::milliseconds{instant.toUnixMilliseconds()}};
        const std::chrono::zoned_time<std::chrono::milliseconds> zoned{
            zone.value(),
            systemTime};
        const auto info = zone.value()->get_info(systemTime);
        auto seconds = offsetSeconds(info.offset);
        if (!seconds)
        {
            return TimeResult<ZonedDateTime>::failure(
                seconds.error().code,
                seconds.error().diagnostic);
        }
        auto offset = UtcOffset::fromSeconds(seconds.value());
        if (!offset)
        {
            return TimeResult<ZonedDateTime>::failure(
                offset.error().code,
                offset.error().diagnostic);
        }
        auto localFields = detail::unixMillisecondsToCalendar(
            zoned.get_local_time().time_since_epoch().count());
        if (!localFields)
        {
            return TimeResult<ZonedDateTime>::failure(
                localFields.error().code,
                localFields.error().diagnostic);
        }
        auto local = fromFields(localFields.value());
        if (!local)
        {
            return TimeResult<ZonedDateTime>::failure(
                local.error().code,
                local.error().diagnostic);
        }
        return TimeResult<ZonedDateTime>::success(ZonedDateTime{
            instant,
            std::move(local).value(),
            timeZone,
            offset.value(),
            info.abbrev,
            info.save != std::chrono::minutes{0}});
    }
    catch (const std::runtime_error& error)
    {
        return TimeResult<ZonedDateTime>::failure(
            TimeErrorCode::TimeZoneDatabaseUnavailable,
            std::string{"timezone conversion failed: "} + error.what());
    }
#else
    return TimeResult<ZonedDateTime>::failure(
        TimeErrorCode::TimeZoneNotSupported,
        "this standard library does not provide C++20 timezone support");
#endif
}

TimeResult<Instant> TimeUtils::toInstant(
    const LocalDateTime& localDateTime,
    const TimeZone& timeZone,
    LocalTimeResolvePolicy policy)
{
    if (timeZone.kind() != TimeZoneKind::Named)
    {
        const int32_t offset = timeZone.configuredOffset()->totalSeconds();
        auto milliseconds = detail::calendarToUnixMilliseconds(
            toFields(localDateTime),
            offset);
        if (!milliseconds)
        {
            return TimeResult<Instant>::failure(
                milliseconds.error().code,
                milliseconds.error().diagnostic);
        }
        return TimeResult<Instant>::success(
            Instant::fromUnixMilliseconds(milliseconds.value()));
    }

#if UCF_TIMEUTILS_HAS_STD_TZDB
    auto zone = locateNamedTimeZone(timeZone.id());
    if (!zone)
    {
        return TimeResult<Instant>::failure(
            zone.error().code,
            zone.error().diagnostic);
    }
    auto localMilliseconds = localTimelineMilliseconds(localDateTime);
    if (!localMilliseconds)
    {
        return TimeResult<Instant>::failure(
            localMilliseconds.error().code,
            localMilliseconds.error().diagnostic);
    }

    try
    {
        const std::chrono::local_time<std::chrono::milliseconds> localTime{
            std::chrono::milliseconds{localMilliseconds.value()}};
        const std::chrono::local_info info = zone.value()->get_info(localTime);
        if (info.result == std::chrono::local_info::unique)
        {
            const auto offset = std::chrono::duration_cast<std::chrono::milliseconds>(
                info.first.offset).count();
            int64_t systemMilliseconds = 0;
            if (!detail::checkedSubtract(
                    localMilliseconds.value(),
                    offset,
                    systemMilliseconds))
            {
                return TimeResult<Instant>::failure(
                    TimeErrorCode::OutOfRange,
                    "timezone conversion exceeds the Instant range");
            }
            return TimeResult<Instant>::success(
                Instant::fromUnixMilliseconds(systemMilliseconds));
        }
        if (info.result == std::chrono::local_info::ambiguous)
        {
            if (policy.ambiguous == AmbiguousTimePolicy::Reject)
            {
                return TimeResult<Instant>::failure(
                    TimeErrorCode::AmbiguousLocalTime,
                    "local date-time occurs twice because the UTC offset moved backward");
            }
            const int64_t firstOffset = std::chrono::duration_cast<std::chrono::milliseconds>(
                info.first.offset).count();
            const int64_t secondOffset = std::chrono::duration_cast<std::chrono::milliseconds>(
                info.second.offset).count();
            int64_t firstCandidate = 0;
            int64_t secondCandidate = 0;
            if (!detail::checkedSubtract(localMilliseconds.value(), firstOffset, firstCandidate)
                || !detail::checkedSubtract(localMilliseconds.value(), secondOffset, secondCandidate))
            {
                return TimeResult<Instant>::failure(
                    TimeErrorCode::OutOfRange,
                    "ambiguous timezone conversion exceeds the Instant range");
            }
            const int64_t earlier = std::min(firstCandidate, secondCandidate);
            const int64_t later = std::max(firstCandidate, secondCandidate);
            return TimeResult<Instant>::success(Instant::fromUnixMilliseconds(
                policy.ambiguous == AmbiguousTimePolicy::Earlier ? earlier : later));
        }

        if (policy.nonexistent == NonexistentTimePolicy::Reject)
        {
            return TimeResult<Instant>::failure(
                TimeErrorCode::NonexistentLocalTime,
                "local date-time does not exist because the UTC offset moved forward");
        }
        const int64_t gap = std::chrono::duration_cast<std::chrono::milliseconds>(
            info.second.offset - info.first.offset).count();
        if (gap <= 0)
        {
            return TimeResult<Instant>::failure(
                TimeErrorCode::TimeZoneDatabaseUnavailable,
                "timezone database reported an invalid nonexistent-time gap");
        }
        int64_t shiftedMilliseconds = 0;
        const bool shifted = policy.nonexistent == NonexistentTimePolicy::ShiftForward
            ? detail::checkedAdd(localMilliseconds.value(), gap, shiftedMilliseconds)
            : detail::checkedSubtract(localMilliseconds.value(), gap, shiftedMilliseconds);
        if (!shifted)
        {
            return TimeResult<Instant>::failure(
                TimeErrorCode::OutOfRange,
                "nonexistent-time adjustment exceeds the local timeline range");
        }
        auto shiftedFields = detail::unixMillisecondsToCalendar(shiftedMilliseconds);
        if (!shiftedFields)
        {
            return TimeResult<Instant>::failure(
                shiftedFields.error().code,
                shiftedFields.error().diagnostic);
        }
        auto shiftedLocal = fromFields(shiftedFields.value());
        if (!shiftedLocal)
        {
            return TimeResult<Instant>::failure(
                shiftedLocal.error().code,
                shiftedLocal.error().diagnostic);
        }
        return toInstant(
            shiftedLocal.value(),
            timeZone,
            LocalTimeResolvePolicy{});
    }
    catch (const std::runtime_error& error)
    {
        return TimeResult<Instant>::failure(
            TimeErrorCode::TimeZoneDatabaseUnavailable,
            std::string{"timezone conversion failed: "} + error.what());
    }
#else
    (void)localDateTime;
    (void)policy;
    return TimeResult<Instant>::failure(
        TimeErrorCode::TimeZoneNotSupported,
        "this standard library does not provide C++20 timezone support");
#endif
}

TimeResult<ZonedDateTime> TimeUtils::now(const TimeZone& timeZone)
{
    return toZonedDateTime(now(), timeZone);
}

TimeResult<LocalDate> TimeUtils::today(const TimeZone& timeZone)
{
    auto zoned = now(timeZone);
    if (!zoned)
    {
        return TimeResult<LocalDate>::failure(
            zoned.error().code,
            zoned.error().diagnostic);
    }
    return TimeResult<LocalDate>::success(zoned.value().localDateTime().date());
}

TimeResult<LocalDateTime> TimeUtils::parseLocalDateTime(
    std::string_view text,
    std::string_view pattern)
{
    if (text.empty() || pattern.empty())
    {
        return TimeResult<LocalDateTime>::failure(
            TimeErrorCode::InvalidFormat,
            "text and portable time pattern must both be non-empty");
    }
    PatternParseState result;
    if (!parsePatternRecursive(text, pattern, 0, 0, PatternParseState{}, result))
    {
        return TimeResult<LocalDateTime>::failure(
            TimeErrorCode::InvalidFormat,
            "text does not exactly match the portable time pattern");
    }
    constexpr unsigned required = YearBit | MonthBit | DayBit
        | HourBit | MinuteBit | SecondBit;
    if ((result.seen & required) != required)
    {
        return TimeResult<LocalDateTime>::failure(
            TimeErrorCode::InvalidFormat,
            "pattern must provide date, hour, minute, and second fields");
    }
    auto dateTime = fromFields(result.fields);
    if (!dateTime)
    {
        return TimeResult<LocalDateTime>::failure(
            TimeErrorCode::InvalidDateTime,
            dateTime.error().diagnostic);
    }
    return dateTime;
}

TimeResult<std::string> TimeUtils::format(
    const LocalDateTime& localDateTime,
    std::string_view pattern)
{
    return formatFields(localDateTime, pattern, nullptr, {});
}

TimeResult<std::string> TimeUtils::format(
    const ZonedDateTime& zonedDateTime,
    std::string_view pattern)
{
    const UtcOffset offset = zonedDateTime.actualOffset();
    return formatFields(
        zonedDateTime.localDateTime(),
        pattern,
        &offset,
        zonedDateTime.abbreviation());
}

TimeResult<std::string> TimeUtils::format(
    Instant instant,
    const TimeZone& timeZone,
    std::string_view pattern)
{
    auto zoned = toZonedDateTime(instant, timeZone);
    if (!zoned)
    {
        return TimeResult<std::string>::failure(
            zoned.error().code,
            zoned.error().diagnostic);
    }
    return format(zoned.value(), pattern);
}

std::string TimeUtils::formatHoursMinutesSeconds(std::chrono::seconds duration)
{
    const uint64_t totalSeconds = magnitude(duration.count());
    const uint64_t hours = totalSeconds / 3600;
    const uint64_t minutes = (totalSeconds % 3600) / 60;
    const uint64_t seconds = totalSeconds % 60;
    return std::string{duration.count() < 0 ? "-" : ""}
        + padAtLeastTwo(hours)
        + ":" + padAtLeastTwo(minutes)
        + ":" + padAtLeastTwo(seconds);
}

std::string TimeUtils::formatMinutesSeconds(std::chrono::seconds duration)
{
    const uint64_t totalSeconds = magnitude(duration.count());
    const uint64_t minutes = totalSeconds / 60;
    const uint64_t seconds = totalSeconds % 60;
    return std::string{duration.count() < 0 ? "-" : ""}
        + padAtLeastTwo(minutes)
        + ":" + padAtLeastTwo(seconds);
}

std::string TimeUtils::formatDuration(std::chrono::milliseconds duration)
{
    const uint64_t totalMilliseconds = magnitude(duration.count());
    const uint64_t totalSeconds = totalMilliseconds / 1000;
    const uint64_t hours = totalSeconds / 3600;
    const uint64_t minutes = (totalSeconds % 3600) / 60;
    const uint64_t seconds = totalSeconds % 60;
    const uint64_t milliseconds = totalMilliseconds % 1000;
    std::ostringstream stream;
    if (duration.count() < 0)
    {
        stream << '-';
    }
    stream << std::setfill('0') << std::setw(2) << hours
           << ':' << std::setw(2) << minutes
           << ':' << std::setw(2) << seconds
           << '.' << std::setw(3) << milliseconds;
    return stream.str();
}

std::string TimeUtils::formatDurationHuman(std::chrono::milliseconds duration)
{
    const uint64_t totalMilliseconds = magnitude(duration.count());
    if (totalMilliseconds == 0)
    {
        return "0 ms";
    }
    if (totalMilliseconds < 1000)
    {
        return std::string{duration.count() < 0 ? "-" : ""}
            + std::to_string(totalMilliseconds)
            + " ms";
    }

    const uint64_t hours = totalMilliseconds / 3'600'000;
    const uint64_t minutes = (totalMilliseconds % 3'600'000) / 60'000;
    const uint64_t seconds = (totalMilliseconds % 60'000) / 1000;
    const uint64_t milliseconds = totalMilliseconds % 1000;
    std::ostringstream stream;
    if (duration.count() < 0)
    {
        stream << '-';
    }
    bool hasPart = false;
    const auto appendPart = [&stream, &hasPart](uint64_t value, std::string_view suffix)
    {
        if (value == 0)
        {
            return;
        }
        if (hasPart)
        {
            stream << ' ';
        }
        stream << value << suffix;
        hasPart = true;
    };
    appendPart(hours, "h");
    appendPart(minutes, "m");
    appendPart(seconds, "s");
    appendPart(milliseconds, "ms");
    return stream.str();
}

} // namespace ucf::utilities

#undef UCF_TIMEUTILS_HAS_STD_TZDB
