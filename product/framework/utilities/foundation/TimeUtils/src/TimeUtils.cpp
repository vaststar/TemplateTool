#include <ucf/Utilities/TimeUtils/TimeUtils.h>

#include <ucf/Utilities/TimeUtils/Instant.h>
#include <ucf/Utilities/TimeUtils/LocalDateTime.h>

#include <chrono>
#include <cstdio>
#include <ctime>

namespace ucf::utilities {

namespace {

std::tm toLocalTm(std::time_t t)
{
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    return tm;
}

int64_t clampToZero(std::chrono::milliseconds d) noexcept
{
    auto ms = d.count();
    return ms < 0 ? 0 : ms;
}

} // namespace

int64_t TimeUtils::getCurrentUTCMilliseconds()
{
    return Instant::now().toUnixMilliseconds();
}

std::time_t TimeUtils::getCurrentUTCSeconds()
{
    return static_cast<std::time_t>(Instant::now().toUnixSeconds());
}

std::string TimeUtils::getCurrentUTCTimeString()
{
    return LocalDateTime::nowUTC().format("%Y-%m-%d %H:%M:%S");
}

std::string TimeUtils::getLocalTimeZoneName()
{
#if defined(_WIN32)
    return std::string{std::chrono::current_zone()->name()};
#else
    std::time_t now = std::time(nullptr);
    auto tm = toLocalTm(now);
    return tm.tm_zone ? std::string(tm.tm_zone) : "UTC";
#endif
}

std::string TimeUtils::formatLocalTime(std::time_t time, const std::string& pattern)
{
    return Instant::fromUnixSeconds(static_cast<int64_t>(time))
        .toLocalDateTime()
        .format(pattern);
}

std::string TimeUtils::formatUTCTime(std::time_t time, const std::string& pattern)
{
    return Instant::fromUnixSeconds(static_cast<int64_t>(time))
        .toUTCDateTime()
        .format(pattern);
}

std::string TimeUtils::formatCurrentLocalTime(const std::string& pattern)
{
    return LocalDateTime::now().format(pattern);
}

std::string TimeUtils::formatCurrentUTCTime(const std::string& pattern)
{
    return LocalDateTime::nowUTC().format(pattern);
}

std::string TimeUtils::formatSecondsToHMS(int totalSeconds)
{
    if (totalSeconds < 0)
    {
        totalSeconds = 0;
    }
    int h = totalSeconds / 3600;
    int m = (totalSeconds % 3600) / 60;
    int s = totalSeconds % 60;
    char buffer[16]{};
    std::snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", h, m, s);
    return buffer;
}

std::string TimeUtils::formatSecondsToMS(int totalSeconds)
{
    if (totalSeconds < 0)
    {
        totalSeconds = 0;
    }
    int m = totalSeconds / 60;
    int s = totalSeconds % 60;
    char buffer[16]{};
    std::snprintf(buffer, sizeof(buffer), "%02d:%02d", m, s);
    return buffer;
}

std::string TimeUtils::formatDuration(std::chrono::milliseconds duration)
{
    int64_t total = clampToZero(duration);
    int64_t h = total / (3600LL * 1000);
    int64_t m = (total / (60LL * 1000)) % 60;
    int64_t s = (total / 1000) % 60;
    int64_t ms = total % 1000;
    char buffer[32]{};
    std::snprintf(buffer, sizeof(buffer),
                  "%02lld:%02lld:%02lld.%03lld",
                  static_cast<long long>(h),
                  static_cast<long long>(m),
                  static_cast<long long>(s),
                  static_cast<long long>(ms));
    return buffer;
}

std::string TimeUtils::formatDurationHuman(std::chrono::milliseconds duration)
{
    int64_t total = clampToZero(duration);
    if (total < 1000)
    {
        char buffer[16]{};
        std::snprintf(buffer, sizeof(buffer), "%lld ms", static_cast<long long>(total));
        return buffer;
    }

    int64_t h = total / (3600LL * 1000);
    int64_t m = (total / (60LL * 1000)) % 60;
    int64_t s = (total / 1000) % 60;

    std::string out;
    char piece[16]{};
    if (h > 0)
    {
        std::snprintf(piece, sizeof(piece), "%lldh", static_cast<long long>(h));
        out += piece;
    }
    if (m > 0 || h > 0)
    {
        if (!out.empty())
        {
            out += ' ';
        }
        std::snprintf(piece, sizeof(piece), "%lldm", static_cast<long long>(m));
        out += piece;
    }
    if (!out.empty())
    {
        out += ' ';
    }
    std::snprintf(piece, sizeof(piece), "%llds", static_cast<long long>(s));
    out += piece;
    return out;
}

} // namespace ucf::utilities
