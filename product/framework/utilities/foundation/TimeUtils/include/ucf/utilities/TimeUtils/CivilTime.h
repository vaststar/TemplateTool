#pragma once

#include <chrono>
#include <compare>
#include <string>
#include <string_view>

#include <ucf/utilities/TimeUtils/TimeResult.h>
#include <ucf/utilities/TimeUtils/TimeUtilsExport.h>

namespace ucf::utilities {

// A proleptic-Gregorian calendar date, independent of timezone.
class TIME_UTILS_API LocalDate final
{
public:
    [[nodiscard]] static TimeResult<LocalDate> create(int year, unsigned month, unsigned day);
    [[nodiscard]] static TimeResult<LocalDate> parseIso(std::string_view text);

    [[nodiscard]] int year() const noexcept;
    [[nodiscard]] unsigned month() const noexcept;
    [[nodiscard]] unsigned day() const noexcept;
    [[nodiscard]] unsigned weekday() const noexcept;
    [[nodiscard]] unsigned dayOfYear() const noexcept;
    [[nodiscard]] std::string toIsoString() const;

    LocalDate& operator+=(std::chrono::days duration);
    LocalDate& operator-=(std::chrono::days duration);

    friend TIME_UTILS_API std::strong_ordering operator<=>(const LocalDate& lhs, const LocalDate& rhs) noexcept;
    friend TIME_UTILS_API bool operator==(const LocalDate& lhs, const LocalDate& rhs) noexcept;
    friend TIME_UTILS_API LocalDate operator+(LocalDate lhs, std::chrono::days rhs);
    friend TIME_UTILS_API LocalDate operator-(LocalDate lhs, std::chrono::days rhs);
    friend TIME_UTILS_API std::chrono::days operator-(const LocalDate& lhs, const LocalDate& rhs) noexcept;

private:
    explicit LocalDate(std::chrono::year_month_day date) noexcept;

private:
    std::chrono::year_month_day mDate;
};

// A wall-clock time-of-day with millisecond precision. Leap seconds and 24:00
// are deliberately not represented.
class TIME_UTILS_API LocalTime final
{
public:
    [[nodiscard]] static TimeResult<LocalTime> create(
        unsigned hour,
        unsigned minute,
        unsigned second,
        unsigned millisecond = 0);
    [[nodiscard]] static TimeResult<LocalTime> parseIso(std::string_view text);

    [[nodiscard]] unsigned hour() const noexcept;
    [[nodiscard]] unsigned minute() const noexcept;
    [[nodiscard]] unsigned second() const noexcept;
    [[nodiscard]] unsigned millisecond() const noexcept;
    [[nodiscard]] std::string toIsoString() const;

    friend TIME_UTILS_API std::strong_ordering operator<=>(const LocalTime& lhs, const LocalTime& rhs) noexcept;
    friend TIME_UTILS_API bool operator==(const LocalTime& lhs, const LocalTime& rhs) noexcept;

private:
    explicit LocalTime(std::chrono::milliseconds sinceMidnight) noexcept;

private:
    std::chrono::milliseconds mSinceMidnight;
};

// Calendar date plus wall-clock time, with no timezone or UTC offset attached.
class TIME_UTILS_API LocalDateTime final
{
public:
    LocalDateTime(LocalDate date, LocalTime time) noexcept;

    [[nodiscard]] static TimeResult<LocalDateTime> parseIso(std::string_view text);

    [[nodiscard]] const LocalDate& date() const noexcept;
    [[nodiscard]] const LocalTime& time() const noexcept;
    [[nodiscard]] std::string toIsoString() const;

    friend TIME_UTILS_API std::strong_ordering operator<=>(const LocalDateTime& lhs, const LocalDateTime& rhs) noexcept;
    friend TIME_UTILS_API bool operator==(const LocalDateTime& lhs, const LocalDateTime& rhs) noexcept;

private:
    LocalDate mDate;
    LocalTime mTime;
};

} // namespace ucf::utilities
