#pragma once

#include <chrono>
#include <ratio>

namespace ucf::utilities::network::detail {

/// Adds a public millisecond option to steady_clock without overflowing the
/// clock representation. Callers validate that delay is non-negative.
[[nodiscard]] inline std::chrono::steady_clock::time_point steadyDeadlineAfter(
    std::chrono::milliseconds delay) noexcept
{
    using Clock = std::chrono::steady_clock;
    static_assert(
        std::ratio_less_equal_v<Clock::period, std::milli>,
        "NetworkTransportUtils requires steady_clock resolution of at least one millisecond");
    const auto now = Clock::now();
    auto remainingDuration = Clock::time_point::max().time_since_epoch();
    if (now.time_since_epoch() > Clock::duration::zero())
    {
        remainingDuration -= now.time_since_epoch();
    }
    const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
        remainingDuration);
    if (delay >= remaining)
    {
        return Clock::time_point::max();
    }
    return now + std::chrono::duration_cast<Clock::duration>(delay);
}

} // namespace ucf::utilities::network::detail
