#pragma once

#include <chrono>
#include <compare>
#include <optional>
#include <string>
#include <string_view>

#include <ucf/Utilities/TimeUtils/LocalDate.h>
#include <ucf/Utilities/UtilitiesCommonFile/UtilitiesExport.h>

namespace ucf::utilities {

// Date + time-of-day with millisecond precision, no timezone attached.
class Utilities_EXPORT LocalDateTime final
{
public:
    LocalDateTime() = default;
    LocalDateTime(LocalDate date,
                  unsigned hour,
                  unsigned minute,
                  unsigned second,
                  unsigned millisecond = 0) noexcept;

    static LocalDateTime now() noexcept;
    static LocalDateTime nowUTC() noexcept;

    // strptime-style pattern; %f is read as 1..3 digit millisecond fraction.
    static std::optional<LocalDateTime> parse(std::string_view text, std::string_view pattern);

    [[nodiscard]] LocalDate date() const noexcept;
    [[nodiscard]] unsigned hour() const noexcept;
    [[nodiscard]] unsigned minute() const noexcept;
    [[nodiscard]] unsigned second() const noexcept;
    [[nodiscard]] unsigned millisecond() const noexcept;

    // strftime-style pattern; %f is replaced with three-digit millisecond.
    [[nodiscard]] std::string format(std::string_view pattern) const;

    friend Utilities_EXPORT std::strong_ordering operator<=>(const LocalDateTime& lhs, const LocalDateTime& rhs) noexcept;
    friend Utilities_EXPORT bool operator==(const LocalDateTime& lhs, const LocalDateTime& rhs) noexcept;

private:
    LocalDate mDate{};
    unsigned mHour{0};
    unsigned mMinute{0};
    unsigned mSecond{0};
    unsigned mMillisecond{0};
};

} // namespace ucf::utilities
