#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <string_view>

#include <ucf/utilities/TimeUtils/Clock.h>
#include <ucf/utilities/TimeUtils/TimeZone.h>
#include <ucf/utilities/TimeUtils/TimeUtilsExport.h>

namespace ucf::utilities {

class TIME_UTILS_API TimeUtils final
{
public:
    [[nodiscard]] static Instant now() noexcept;

    // Named-zone support is compile-time dependent on the standard library's
    // C++20 tzdb implementation. UTC and fixed offsets are always supported.
    [[nodiscard]] static bool supportsNamedTimeZones() noexcept;
    // Accepts UTC or an IANA region ID. Abbreviations such as EST are rejected.
    [[nodiscard]] static TimeResult<TimeZone> findTimeZone(std::string_view id);
    [[nodiscard]] static TimeResult<TimeZone> systemTimeZone();
    [[nodiscard]] static std::optional<std::string> timeZoneDatabaseVersion();

    [[nodiscard]] static TimeResult<ZonedDateTime> toZonedDateTime(
        Instant instant,
        const TimeZone& timeZone);
    [[nodiscard]] static TimeResult<Instant> toInstant(
        const LocalDateTime& localDateTime,
        const TimeZone& timeZone,
        LocalTimeResolvePolicy policy = {});
    [[nodiscard]] static TimeResult<ZonedDateTime> now(const TimeZone& timeZone);
    [[nodiscard]] static TimeResult<LocalDate> today(const TimeZone& timeZone);

    // Portable pattern subset: %Y %m %d %H %M %S %f %z %Z %%.
    // LocalDateTime parsing/formatting rejects timezone directives.
    [[nodiscard]] static TimeResult<LocalDateTime> parseLocalDateTime(
        std::string_view text,
        std::string_view pattern);
    [[nodiscard]] static TimeResult<std::string> format(
        const LocalDateTime& localDateTime,
        std::string_view pattern);
    [[nodiscard]] static TimeResult<std::string> format(
        const ZonedDateTime& zonedDateTime,
        std::string_view pattern);
    [[nodiscard]] static TimeResult<std::string> format(
        Instant instant,
        const TimeZone& timeZone,
        std::string_view pattern);

    [[nodiscard]] static std::string formatHoursMinutesSeconds(std::chrono::seconds duration);
    [[nodiscard]] static std::string formatMinutesSeconds(std::chrono::seconds duration);
    [[nodiscard]] static std::string formatDuration(std::chrono::milliseconds duration);
    [[nodiscard]] static std::string formatDurationHuman(std::chrono::milliseconds duration);

    TimeUtils() = delete;
    TimeUtils(const TimeUtils&) = delete;
    TimeUtils& operator=(const TimeUtils&) = delete;
    TimeUtils(TimeUtils&&) = delete;
    TimeUtils& operator=(TimeUtils&&) = delete;
    ~TimeUtils() = delete;
};

} // namespace ucf::utilities
