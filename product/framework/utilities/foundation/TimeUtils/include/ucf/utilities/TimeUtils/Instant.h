#pragma once

#include <chrono>
#include <compare>
#include <cstdint>
#include <ctime>
#include <string>
#include <string_view>

#include <ucf/utilities/TimeUtils/TimeResult.h>
#include <ucf/utilities/TimeUtils/TimeUtilsExport.h>

namespace ucf::utilities {

// An absolute point on the Unix timeline, stored with millisecond precision.
// Instant deliberately has no calendar or timezone responsibilities.
class TIME_UTILS_API Instant final
{
public:
    using Clock = std::chrono::system_clock;
    using TimePoint = std::chrono::time_point<Clock, std::chrono::milliseconds>;

    Instant() = default;

    [[nodiscard]] static Instant fromUnixMilliseconds(int64_t milliseconds) noexcept;
    [[nodiscard]] static TimeResult<Instant> fromUnixSeconds(int64_t seconds);
    [[nodiscard]] static TimeResult<Instant> fromTimeT(std::time_t time);
    [[nodiscard]] static TimeResult<Instant> parseRfc3339(std::string_view text);

    [[nodiscard]] int64_t toUnixMilliseconds() const noexcept;
    // Whole seconds are floored: -1 ms is -1 s.
    [[nodiscard]] int64_t toUnixSeconds() const noexcept;
    [[nodiscard]] TimeResult<std::time_t> toTimeT() const;
    // UTC representation with an explicit three-digit fraction.
    [[nodiscard]] TimeResult<std::string> toRfc3339() const;

    // Arithmetic throws std::overflow_error rather than wrapping the timeline.
    Instant& operator+=(std::chrono::milliseconds duration);
    Instant& operator-=(std::chrono::milliseconds duration);

    friend TIME_UTILS_API std::strong_ordering operator<=>(const Instant& lhs, const Instant& rhs) noexcept;
    friend TIME_UTILS_API bool operator==(const Instant& lhs, const Instant& rhs) noexcept;
    friend TIME_UTILS_API Instant operator+(Instant lhs, std::chrono::milliseconds rhs);
    friend TIME_UTILS_API Instant operator-(Instant lhs, std::chrono::milliseconds rhs);
    friend TIME_UTILS_API std::chrono::milliseconds operator-(const Instant& lhs, const Instant& rhs);

private:
    explicit Instant(TimePoint timePoint) noexcept;

private:
    TimePoint mTimePoint{};
};

} // namespace ucf::utilities
