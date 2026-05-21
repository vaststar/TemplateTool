#pragma once

#include <chrono>
#include <compare>
#include <optional>
#include <string>
#include <string_view>

#include <ucf/Utilities/UtilitiesCommonFile/UtilitiesExport.h>

namespace ucf::utilities {

// Calendar date with no time-of-day and no timezone.
class Utilities_EXPORT LocalDate final
{
public:
    LocalDate() = default;
    LocalDate(int year, unsigned month, unsigned day) noexcept;

    static LocalDate today() noexcept;
    static LocalDate todayUTC() noexcept;

    // "YYYY-MM-DD".
    static std::optional<LocalDate> parse(std::string_view text);

    [[nodiscard]] bool isValid() const noexcept;
    [[nodiscard]] int year() const noexcept;
    [[nodiscard]] unsigned month() const noexcept;
    [[nodiscard]] unsigned day() const noexcept;
    [[nodiscard]] unsigned dayOfYear() const noexcept;

    // ISO 8601 weekday: 1=Monday .. 7=Sunday.
    [[nodiscard]] unsigned weekday() const noexcept;

    // "YYYY-MM-DD".
    [[nodiscard]] std::string toString() const;

    LocalDate& operator+=(std::chrono::days d) noexcept;
    LocalDate& operator-=(std::chrono::days d) noexcept;

    friend Utilities_EXPORT std::strong_ordering operator<=>(const LocalDate& lhs, const LocalDate& rhs) noexcept;
    friend Utilities_EXPORT bool operator==(const LocalDate& lhs, const LocalDate& rhs) noexcept;
    friend Utilities_EXPORT LocalDate operator+(LocalDate lhs, std::chrono::days rhs) noexcept;
    friend Utilities_EXPORT LocalDate operator-(LocalDate lhs, std::chrono::days rhs) noexcept;
    friend Utilities_EXPORT std::chrono::days operator-(const LocalDate& lhs, const LocalDate& rhs) noexcept;

private:
    std::chrono::year_month_day mYmd{};
};

} // namespace ucf::utilities
