#include <ucf/utilities/TimeUtils/Clock.h>

#include <chrono>

namespace ucf::utilities {

const SystemClock& SystemClock::instance() noexcept
{
    static const SystemClock clock;
    return clock;
}

Instant SystemClock::now() const noexcept
{
    const auto milliseconds = std::chrono::floor<std::chrono::milliseconds>(
        std::chrono::system_clock::now());
    return Instant::fromUnixMilliseconds(milliseconds.time_since_epoch().count());
}

} // namespace ucf::utilities
