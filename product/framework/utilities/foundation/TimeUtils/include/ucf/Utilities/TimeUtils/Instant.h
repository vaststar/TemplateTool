#pragma once

#include <chrono>
#include <compare>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

#include <ucf/Utilities/TimeUtils/TimeUtilsExport.h>

namespace ucf::utilities {

class LocalDateTime;

// Absolute moment in time, system_clock based, millisecond precision.
class TIME_UTILS_API Instant final
{
public:
    using Clock = std::chrono::system_clock;
    using TimePoint = std::chrono::time_point<Clock, std::chrono::milliseconds>;

    Instant() = default;
    explicit Instant(TimePoint tp) noexcept;

    static Instant now() noexcept;
    static Instant fromUnixMilliseconds(int64_t ms) noexcept;
    static Instant fromUnixSeconds(int64_t s) noexcept;

    // "YYYY-MM-DDTHH:MM:SS[.fff]Z".
    static std::optional<Instant> parseISO8601(std::string_view text);

    [[nodiscard]] TimePoint timePoint() const noexcept;
    [[nodiscard]] int64_t toUnixMilliseconds() const noexcept;
    [[nodiscard]] int64_t toUnixSeconds() const noexcept;

    // "YYYY-MM-DDTHH:MM:SS.fffZ".
    [[nodiscard]] std::string toISO8601() const;

    [[nodiscard]] LocalDateTime toUTCDateTime() const;
    [[nodiscard]] LocalDateTime toLocalDateTime() const;

    Instant& operator+=(std::chrono::milliseconds d) noexcept;
    Instant& operator-=(std::chrono::milliseconds d) noexcept;

    friend TIME_UTILS_API std::strong_ordering operator<=>(const Instant& lhs, const Instant& rhs) noexcept;
    friend TIME_UTILS_API bool operator==(const Instant& lhs, const Instant& rhs) noexcept;
    friend TIME_UTILS_API Instant operator+(Instant lhs, std::chrono::milliseconds rhs) noexcept;
    friend TIME_UTILS_API Instant operator-(Instant lhs, std::chrono::milliseconds rhs) noexcept;
    friend TIME_UTILS_API std::chrono::milliseconds operator-(const Instant& lhs, const Instant& rhs) noexcept;

private:
    TimePoint mTp{};
};

} // namespace ucf::utilities
