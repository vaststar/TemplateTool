#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

#include <ucf/utilities/TimeUtils/CivilTime.h>
#include <ucf/utilities/TimeUtils/Instant.h>
#include <ucf/utilities/TimeUtils/TimeResult.h>
#include <ucf/utilities/TimeUtils/TimeUtilsExport.h>

namespace ucf::utilities {

class TimeUtils;

class TIME_UTILS_API UtcOffset final
{
public:
    [[nodiscard]] static UtcOffset utc() noexcept;
    [[nodiscard]] static TimeResult<UtcOffset> fromSeconds(int32_t seconds);
    // minutes is a magnitude when hours is nonzero. A negative minutes value
    // represents a negative sub-hour offset, for example (0, -30).
    [[nodiscard]] static TimeResult<UtcOffset> fromHoursMinutes(int hours, int minutes);

    [[nodiscard]] int32_t totalSeconds() const noexcept;
    [[nodiscard]] std::string toString() const;

    friend TIME_UTILS_API bool operator==(const UtcOffset& lhs, const UtcOffset& rhs) noexcept;

private:
    explicit UtcOffset(int32_t seconds) noexcept;

private:
    int32_t mSeconds{0};
};

enum class TimeZoneKind
{
    Utc,
    FixedOffset,
    Named
};

class TIME_UTILS_API TimeZone final
{
public:
    [[nodiscard]] static TimeZone utc();
    [[nodiscard]] static TimeZone fixedOffset(UtcOffset offset);

    [[nodiscard]] TimeZoneKind kind() const noexcept;
    [[nodiscard]] std::string_view id() const noexcept;
    [[nodiscard]] std::optional<UtcOffset> configuredOffset() const noexcept;

    friend TIME_UTILS_API bool operator==(const TimeZone& lhs, const TimeZone& rhs) noexcept;

private:
    friend class TimeUtils;

    TimeZone(TimeZoneKind kind, std::string id, UtcOffset offset);
    [[nodiscard]] static TimeZone named(std::string canonicalId);

private:
    TimeZoneKind mKind;
    std::string mId;
    UtcOffset mOffset;
};

enum class AmbiguousTimePolicy
{
    Reject,
    // Choose the chronologically earlier of the two matching Instants.
    Earlier,
    // Choose the chronologically later of the two matching Instants.
    Later
};

enum class NonexistentTimePolicy
{
    Reject,
    // Move the local value forward by the transition gap before resolving it.
    ShiftForward,
    // Move the local value backward by the transition gap before resolving it.
    ShiftBackward
};

struct LocalTimeResolvePolicy final
{
    AmbiguousTimePolicy ambiguous{AmbiguousTimePolicy::Reject};
    NonexistentTimePolicy nonexistent{NonexistentTimePolicy::Reject};
};

class TIME_UTILS_API ZonedDateTime final
{
public:
    [[nodiscard]] const Instant& instant() const noexcept;
    [[nodiscard]] const LocalDateTime& localDateTime() const noexcept;
    [[nodiscard]] const TimeZone& timeZone() const noexcept;
    // For a named timezone this is the offset that applied at instant(), not
    // a fixed configuration value.
    [[nodiscard]] UtcOffset actualOffset() const noexcept;
    [[nodiscard]] std::string_view abbreviation() const noexcept;
    [[nodiscard]] bool isDaylightSavingTime() const noexcept;

private:
    friend class TimeUtils;

    ZonedDateTime(
        Instant instant,
        LocalDateTime localDateTime,
        TimeZone timeZone,
        UtcOffset actualOffset,
        std::string abbreviation,
        bool daylightSavingTime);

private:
    Instant mInstant;
    LocalDateTime mLocalDateTime;
    TimeZone mTimeZone;
    UtcOffset mActualOffset;
    std::string mAbbreviation;
    bool mDaylightSavingTime{false};
};

} // namespace ucf::utilities
