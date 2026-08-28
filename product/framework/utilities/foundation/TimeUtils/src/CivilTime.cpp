#include <ucf/utilities/TimeUtils/CivilTime.h>

#include "TimeConversion.h"

#include <cstdio>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace ucf::utilities {

namespace {

TimeResult<LocalTime> parseLocalTime(std::string_view text)
{
    const bool hasFraction = text.size() >= 10 && text.size() <= 12;
    if ((text.size() != 8 && !hasFraction)
        || text[2] != ':'
        || text[5] != ':'
        || (hasFraction && text[8] != '.'))
    {
        return TimeResult<LocalTime>::failure(
            TimeErrorCode::InvalidFormat,
            "local time must match HH:MM:SS[.fff]");
    }

    unsigned hour = 0;
    unsigned minute = 0;
    unsigned second = 0;
    unsigned millisecond = 0;
    if (!detail::parseUnsigned(text.substr(0, 2), hour)
        || !detail::parseUnsigned(text.substr(3, 2), minute)
        || !detail::parseUnsigned(text.substr(6, 2), second))
    {
        return TimeResult<LocalTime>::failure(
            TimeErrorCode::InvalidFormat,
            "local time contains a non-numeric field");
    }
    if (hasFraction)
    {
        const auto fraction = text.substr(9);
        if (!detail::parseUnsigned(fraction, millisecond))
        {
            return TimeResult<LocalTime>::failure(
                TimeErrorCode::InvalidFormat,
                "fractional second contains a non-numeric field");
        }
        if (fraction.size() == 1)
        {
            millisecond *= 100;
        }
        else if (fraction.size() == 2)
        {
            millisecond *= 10;
        }
    }
    return LocalTime::create(hour, minute, second, millisecond);
}

} // namespace

LocalDate::LocalDate(std::chrono::year_month_day date) noexcept
    : mDate{date}
{
}

TimeResult<LocalDate> LocalDate::create(int year, unsigned month, unsigned day)
{
    if (year < static_cast<int>(std::chrono::year::min())
        || year > static_cast<int>(std::chrono::year::max()))
    {
        return TimeResult<LocalDate>::failure(
            TimeErrorCode::OutOfRange,
            "calendar year is outside std::chrono::year range");
    }
    if (month < 1 || month > 12 || day < 1 || day > 31)
    {
        return TimeResult<LocalDate>::failure(
            TimeErrorCode::InvalidDate,
            "calendar month or day is outside its valid range");
    }
    const std::chrono::year_month_day value{
        std::chrono::year{year},
        std::chrono::month{month},
        std::chrono::day{day}};
    if (!value.ok())
    {
        return TimeResult<LocalDate>::failure(
            TimeErrorCode::InvalidDate,
            "invalid proleptic-Gregorian date");
    }
    return TimeResult<LocalDate>::success(LocalDate{value});
}

TimeResult<LocalDate> LocalDate::parseIso(std::string_view text)
{
    if (text.size() < 10
        || text[text.size() - 6] != '-'
        || text[text.size() - 3] != '-')
    {
        return TimeResult<LocalDate>::failure(
            TimeErrorCode::InvalidFormat,
            "local date must match YYYY-MM-DD or a signed expanded year");
    }

    const auto yearText = text.substr(0, text.size() - 6);
    const bool expanded = yearText.front() == '+' || yearText.front() == '-';
    if ((!expanded && yearText.size() != 4)
        || (expanded && yearText.size() < 5))
    {
        return TimeResult<LocalDate>::failure(
            TimeErrorCode::InvalidFormat,
            "ISO year must have four digits or a sign with at least four digits");
    }
    int year = 0;
    unsigned month = 0;
    unsigned day = 0;
    const auto numericYear = yearText.front() == '+' ? yearText.substr(1) : yearText;
    if (!detail::parseYear(numericYear, year)
        || !detail::parseUnsigned(text.substr(text.size() - 5, 2), month)
        || !detail::parseUnsigned(text.substr(text.size() - 2, 2), day))
    {
        return TimeResult<LocalDate>::failure(
            TimeErrorCode::InvalidFormat,
            "local date contains a non-numeric field");
    }
    return create(year, month, day);
}

int LocalDate::year() const noexcept
{
    return static_cast<int>(mDate.year());
}

unsigned LocalDate::month() const noexcept
{
    return static_cast<unsigned>(mDate.month());
}

unsigned LocalDate::day() const noexcept
{
    return static_cast<unsigned>(mDate.day());
}

unsigned LocalDate::weekday() const noexcept
{
    return std::chrono::weekday{std::chrono::sys_days{mDate}}.iso_encoding();
}

unsigned LocalDate::dayOfYear() const noexcept
{
    const auto firstDay = std::chrono::sys_days{mDate.year() / std::chrono::January / 1};
    return static_cast<unsigned>((std::chrono::sys_days{mDate} - firstDay).count() + 1);
}

std::string LocalDate::toIsoString() const
{
    std::ostringstream stream;
    stream << std::setfill('0');
    if (year() >= 0 && year() <= 9999)
    {
        stream << std::setw(4) << year();
    }
    else
    {
        const int64_t signedYear = year();
        const uint64_t magnitude = signedYear < 0
            ? static_cast<uint64_t>(-signedYear)
            : static_cast<uint64_t>(signedYear);
        stream << (signedYear < 0 ? '-' : '+')
               << std::setw(4) << magnitude;
    }
    stream << '-' << std::setw(2) << month()
           << '-' << std::setw(2) << day();
    return stream.str();
}

LocalDate& LocalDate::operator+=(std::chrono::days duration)
{
    const int64_t current = std::chrono::sys_days{mDate}.time_since_epoch().count();
    int64_t target = 0;
    if (!detail::checkedAdd(current, duration.count(), target))
    {
        throw std::out_of_range{"LocalDate addition overflow"};
    }
    constexpr auto minimumDay = std::chrono::sys_days{
        std::chrono::year::min() / std::chrono::January / 1};
    constexpr auto maximumDay = std::chrono::sys_days{
        std::chrono::year::max() / std::chrono::December / 31};
    if (target < minimumDay.time_since_epoch().count()
        || target > maximumDay.time_since_epoch().count())
    {
        throw std::out_of_range{"LocalDate result is outside std::chrono::year range"};
    }
    const auto targetDay = std::chrono::sys_days{
        std::chrono::days{static_cast<std::chrono::days::rep>(target)}};
    mDate = std::chrono::year_month_day{targetDay};
    return *this;
}

LocalDate& LocalDate::operator-=(std::chrono::days duration)
{
    const int64_t current = std::chrono::sys_days{mDate}.time_since_epoch().count();
    int64_t target = 0;
    if (!detail::checkedSubtract(current, duration.count(), target))
    {
        throw std::out_of_range{"LocalDate subtraction overflow"};
    }
    constexpr auto minimumDay = std::chrono::sys_days{
        std::chrono::year::min() / std::chrono::January / 1};
    constexpr auto maximumDay = std::chrono::sys_days{
        std::chrono::year::max() / std::chrono::December / 31};
    if (target < minimumDay.time_since_epoch().count()
        || target > maximumDay.time_since_epoch().count())
    {
        throw std::out_of_range{"LocalDate result is outside std::chrono::year range"};
    }
    const auto targetDay = std::chrono::sys_days{
        std::chrono::days{static_cast<std::chrono::days::rep>(target)}};
    mDate = std::chrono::year_month_day{targetDay};
    return *this;
}

std::strong_ordering operator<=>(const LocalDate& lhs, const LocalDate& rhs) noexcept
{
    return std::chrono::sys_days{lhs.mDate} <=> std::chrono::sys_days{rhs.mDate};
}

bool operator==(const LocalDate& lhs, const LocalDate& rhs) noexcept
{
    return lhs.mDate == rhs.mDate;
}

LocalDate operator+(LocalDate lhs, std::chrono::days rhs)
{
    lhs += rhs;
    return lhs;
}

LocalDate operator-(LocalDate lhs, std::chrono::days rhs)
{
    lhs -= rhs;
    return lhs;
}

std::chrono::days operator-(const LocalDate& lhs, const LocalDate& rhs) noexcept
{
    return std::chrono::sys_days{lhs.mDate} - std::chrono::sys_days{rhs.mDate};
}

LocalTime::LocalTime(std::chrono::milliseconds sinceMidnight) noexcept
    : mSinceMidnight{sinceMidnight}
{
}

TimeResult<LocalTime> LocalTime::create(
    unsigned hour,
    unsigned minute,
    unsigned second,
    unsigned millisecond)
{
    if (hour >= 24 || minute >= 60 || second >= 60 || millisecond >= 1000)
    {
        return TimeResult<LocalTime>::failure(
            TimeErrorCode::InvalidTime,
            "time-of-day is outside its valid range");
    }
    const auto value = std::chrono::hours{hour}
        + std::chrono::minutes{minute}
        + std::chrono::seconds{second}
        + std::chrono::milliseconds{millisecond};
    return TimeResult<LocalTime>::success(
        LocalTime{std::chrono::duration_cast<std::chrono::milliseconds>(value)});
}

TimeResult<LocalTime> LocalTime::parseIso(std::string_view text)
{
    return parseLocalTime(text);
}

unsigned LocalTime::hour() const noexcept
{
    return static_cast<unsigned>(
        std::chrono::duration_cast<std::chrono::hours>(mSinceMidnight).count());
}

unsigned LocalTime::minute() const noexcept
{
    return static_cast<unsigned>(
        std::chrono::duration_cast<std::chrono::minutes>(mSinceMidnight).count() % 60);
}

unsigned LocalTime::second() const noexcept
{
    return static_cast<unsigned>(
        std::chrono::duration_cast<std::chrono::seconds>(mSinceMidnight).count() % 60);
}

unsigned LocalTime::millisecond() const noexcept
{
    return static_cast<unsigned>(mSinceMidnight.count() % 1000);
}

std::string LocalTime::toIsoString() const
{
    char buffer[20]{};
    const int length = std::snprintf(
        buffer,
        sizeof(buffer),
        "%02u:%02u:%02u.%03u",
        hour(),
        minute(),
        second(),
        millisecond());
    return std::string{buffer, static_cast<std::size_t>(length)};
}

std::strong_ordering operator<=>(const LocalTime& lhs, const LocalTime& rhs) noexcept
{
    return lhs.mSinceMidnight <=> rhs.mSinceMidnight;
}

bool operator==(const LocalTime& lhs, const LocalTime& rhs) noexcept
{
    return lhs.mSinceMidnight == rhs.mSinceMidnight;
}

LocalDateTime::LocalDateTime(LocalDate date, LocalTime time) noexcept
    : mDate{date}
    , mTime{time}
{
}

TimeResult<LocalDateTime> LocalDateTime::parseIso(std::string_view text)
{
    const auto separator = text.find('T');
    if (separator == std::string_view::npos
        || separator != text.rfind('T'))
    {
        return TimeResult<LocalDateTime>::failure(
            TimeErrorCode::InvalidFormat,
            "local date-time must match YYYY-MM-DDTHH:MM:SS[.fff]");
    }
    auto date = LocalDate::parseIso(text.substr(0, separator));
    if (!date)
    {
        return TimeResult<LocalDateTime>::failure(
            date.error().code,
            date.error().diagnostic);
    }
    auto time = LocalTime::parseIso(text.substr(separator + 1));
    if (!time)
    {
        return TimeResult<LocalDateTime>::failure(
            time.error().code,
            time.error().diagnostic);
    }
    return TimeResult<LocalDateTime>::success(
        LocalDateTime{std::move(date).value(), std::move(time).value()});
}

const LocalDate& LocalDateTime::date() const noexcept
{
    return mDate;
}

const LocalTime& LocalDateTime::time() const noexcept
{
    return mTime;
}

std::string LocalDateTime::toIsoString() const
{
    return mDate.toIsoString() + "T" + mTime.toIsoString();
}

std::strong_ordering operator<=>(const LocalDateTime& lhs, const LocalDateTime& rhs) noexcept
{
    if (const auto dateOrder = lhs.mDate <=> rhs.mDate; dateOrder != 0)
    {
        return dateOrder;
    }
    return lhs.mTime <=> rhs.mTime;
}

bool operator==(const LocalDateTime& lhs, const LocalDateTime& rhs) noexcept
{
    return lhs.mDate == rhs.mDate && lhs.mTime == rhs.mTime;
}

} // namespace ucf::utilities
