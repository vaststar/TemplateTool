#include <chrono>
#include <cstdio>
#include <ctime>
#include <iomanip>
#include <sstream>

#include <ucf/Utilities/TimeUtils/TimeUtils.h>

namespace {

// Thread-safe local time conversion (platform-independent helper)
std::tm toLocalTime(std::time_t time)
{
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &time);
#else
    localtime_r(&time, &tm);
#endif
    return tm;
}

// Thread-safe UTC time conversion (platform-independent helper)
std::tm toUTCTime(std::time_t time)
{
    std::tm tm{};
#if defined(_WIN32)
    gmtime_s(&tm, &time);
#else
    gmtime_r(&time, &tm);
#endif
    return tm;
}

} // anonymous namespace

namespace ucf::utilities {

// ---- Current time as numeric values ----

int64_t TimeUtils::getCurrentUTCMilliseconds()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

std::time_t TimeUtils::getCurrentUTCSeconds()
{
    return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}

// ---- Current time as fixed-format strings ----

std::string TimeUtils::getCurrentUTCTimeString()
{
    return formatCurrentUTCTime("%Y-%m-%d %H:%M:%S");
}

std::string TimeUtils::getLocalTimeZoneName()
{
#if defined(_WIN32)
    // Windows MSVC supports C++20 timezone library
    return std::string{std::chrono::current_zone()->name()};
#else
    // macOS/Linux: libc++ doesn't support current_zone(), use POSIX API
    std::time_t now = std::time(nullptr);
    auto localTm = toLocalTime(now);
    return localTm.tm_zone ? std::string(localTm.tm_zone) : "UTC";
#endif
}

// ---- Custom format (caller provides time_t) ----

std::string TimeUtils::formatLocalTime(std::time_t time, const std::string& pattern)
{
    if (pattern.empty()) {
        return {};
    }
    auto tm = toLocalTime(time);
    char buffer[128]{};
    size_t len = std::strftime(buffer, sizeof(buffer), pattern.c_str(), &tm);
    if (len == 0) {
        // Fallback to safe default format on invalid pattern or overflow
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);
    }
    return buffer;
}

std::string TimeUtils::formatUTCTime(std::time_t time, const std::string& pattern)
{
    if (pattern.empty()) {
        return {};
    }
    auto tm = toUTCTime(time);
    char buffer[128]{};
    size_t len = std::strftime(buffer, sizeof(buffer), pattern.c_str(), &tm);
    if (len == 0) {
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);
    }
    return buffer;
}

// ---- Custom format (auto-captures current time) ----

std::string TimeUtils::formatCurrentLocalTime(const std::string& pattern)
{
    return formatLocalTime(getCurrentUTCSeconds(), pattern);
}

std::string TimeUtils::formatCurrentUTCTime(const std::string& pattern)
{
    return formatUTCTime(getCurrentUTCSeconds(), pattern);
}

// ---- Duration formatting ----

std::string TimeUtils::formatSecondsToHMS(int totalSeconds)
{
    if (totalSeconds < 0) {
        totalSeconds = 0;
    }
    int h = totalSeconds / 3600;
    int m = (totalSeconds % 3600) / 60;
    int s = totalSeconds % 60;
    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", h, m, s);
    return buffer;
}

std::string TimeUtils::formatSecondsToMS(int totalSeconds)
{
    if (totalSeconds < 0) {
        totalSeconds = 0;
    }
    int m = totalSeconds / 60;
    int s = totalSeconds % 60;
    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "%02d:%02d", m, s);
    return buffer;
}

} // namespace ucf::utilities
