#include <ucf/utilities/TimeUtils/TimeZone.h>

#include <cstdio>
#include <cstdlib>
#include <utility>

namespace ucf::utilities {

namespace {

constexpr int32_t kMaximumUtcOffsetSeconds = 18 * 60 * 60;

} // namespace

UtcOffset::UtcOffset(int32_t seconds) noexcept
    : mSeconds{seconds}
{
}

UtcOffset UtcOffset::utc() noexcept
{
    return UtcOffset{0};
}

TimeResult<UtcOffset> UtcOffset::fromSeconds(int32_t seconds)
{
    if (seconds < -kMaximumUtcOffsetSeconds || seconds > kMaximumUtcOffsetSeconds)
    {
        return TimeResult<UtcOffset>::failure(
            TimeErrorCode::OutOfRange,
            "UTC offset must be within +/-18:00");
    }
    return TimeResult<UtcOffset>::success(UtcOffset{seconds});
}

TimeResult<UtcOffset> UtcOffset::fromHoursMinutes(int hours, int minutes)
{
    if (hours < -18 || hours > 18 || minutes < -59 || minutes > 59
        || (hours > 0 && minutes < 0)
        || (hours < 0 && minutes < 0))
    {
        return TimeResult<UtcOffset>::failure(
            TimeErrorCode::OutOfRange,
            "UTC offset hours and minutes have an invalid range or sign");
    }

    int seconds = 0;
    if (hours < 0)
    {
        seconds = hours * 3600 - minutes * 60;
    }
    else
    {
        seconds = hours * 3600 + minutes * 60;
    }
    return fromSeconds(static_cast<int32_t>(seconds));
}

int32_t UtcOffset::totalSeconds() const noexcept
{
    return mSeconds;
}

std::string UtcOffset::toString() const
{
    const int64_t signedSeconds = mSeconds;
    const uint64_t magnitude = signedSeconds < 0
        ? static_cast<uint64_t>(-signedSeconds)
        : static_cast<uint64_t>(signedSeconds);
    const auto hours = static_cast<unsigned>(magnitude / 3600);
    const auto minutes = static_cast<unsigned>((magnitude % 3600) / 60);
    const auto seconds = static_cast<unsigned>(magnitude % 60);

    char buffer[16]{};
    const int length = seconds == 0
        ? std::snprintf(
              buffer,
              sizeof(buffer),
              "%c%02u:%02u",
              mSeconds < 0 ? '-' : '+',
              hours,
              minutes)
        : std::snprintf(
              buffer,
              sizeof(buffer),
              "%c%02u:%02u:%02u",
              mSeconds < 0 ? '-' : '+',
              hours,
              minutes,
              seconds);
    return std::string{buffer, static_cast<std::size_t>(length)};
}

bool operator==(const UtcOffset& lhs, const UtcOffset& rhs) noexcept
{
    return lhs.mSeconds == rhs.mSeconds;
}

TimeZone::TimeZone(TimeZoneKind kind, std::string id, UtcOffset offset)
    : mKind{kind}
    , mId{std::move(id)}
    , mOffset{offset}
{
}

TimeZone TimeZone::utc()
{
    return TimeZone{TimeZoneKind::Utc, "UTC", UtcOffset::utc()};
}

TimeZone TimeZone::fixedOffset(UtcOffset offset)
{
    return TimeZone{TimeZoneKind::FixedOffset, offset.toString(), offset};
}

TimeZone TimeZone::named(std::string canonicalId)
{
    return TimeZone{
        TimeZoneKind::Named,
        std::move(canonicalId),
        UtcOffset::utc()};
}

TimeZoneKind TimeZone::kind() const noexcept
{
    return mKind;
}

std::string_view TimeZone::id() const noexcept
{
    return mId;
}

std::optional<UtcOffset> TimeZone::configuredOffset() const noexcept
{
    if (mKind == TimeZoneKind::Named)
    {
        return std::nullopt;
    }
    return mOffset;
}

bool operator==(const TimeZone& lhs, const TimeZone& rhs) noexcept
{
    return lhs.mKind == rhs.mKind
        && lhs.mId == rhs.mId
        && lhs.mOffset == rhs.mOffset;
}

ZonedDateTime::ZonedDateTime(
    Instant instant,
    LocalDateTime localDateTime,
    TimeZone timeZone,
    UtcOffset actualOffset,
    std::string abbreviation,
    bool daylightSavingTime)
    : mInstant{instant}
    , mLocalDateTime{std::move(localDateTime)}
    , mTimeZone{std::move(timeZone)}
    , mActualOffset{actualOffset}
    , mAbbreviation{std::move(abbreviation)}
    , mDaylightSavingTime{daylightSavingTime}
{
}

const Instant& ZonedDateTime::instant() const noexcept
{
    return mInstant;
}

const LocalDateTime& ZonedDateTime::localDateTime() const noexcept
{
    return mLocalDateTime;
}

const TimeZone& ZonedDateTime::timeZone() const noexcept
{
    return mTimeZone;
}

UtcOffset ZonedDateTime::actualOffset() const noexcept
{
    return mActualOffset;
}

std::string_view ZonedDateTime::abbreviation() const noexcept
{
    return mAbbreviation;
}

bool ZonedDateTime::isDaylightSavingTime() const noexcept
{
    return mDaylightSavingTime;
}

} // namespace ucf::utilities
