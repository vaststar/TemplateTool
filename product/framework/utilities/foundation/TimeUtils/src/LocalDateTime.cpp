#include <ucf/Utilities/TimeUtils/LocalDateTime.h>

#include <array>
#include <charconv>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <system_error>

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

std::tm toUTCTm(std::time_t t)
{
    std::tm tm{};
#if defined(_WIN32)
    gmtime_s(&tm, &t);
#else
    gmtime_r(&t, &tm);
#endif
    return tm;
}

LocalDateTime fromTmAndMs(const std::tm& tm, unsigned ms) noexcept
{
    LocalDate d{tm.tm_year + 1900, static_cast<unsigned>(tm.tm_mon + 1), static_cast<unsigned>(tm.tm_mday)};
    return LocalDateTime{d,
                         static_cast<unsigned>(tm.tm_hour),
                         static_cast<unsigned>(tm.tm_min),
                         static_cast<unsigned>(tm.tm_sec),
                         ms};
}

void splitNowMs(std::time_t& outSec, unsigned& outMs) noexcept
{
    auto tp = std::chrono::system_clock::now();
    auto secs = std::chrono::time_point_cast<std::chrono::seconds>(tp);
    outSec = std::chrono::system_clock::to_time_t(secs);
    outMs = static_cast<unsigned>(
        std::chrono::duration_cast<std::chrono::milliseconds>(tp - secs).count());
}

// Render strftime to a string, with sane fallback on bad pattern.
std::string strftimeToString(const std::tm& tm, const std::string& pattern)
{
    if (pattern.empty())
    {
        return {};
    }
    char buffer[256]{};
    std::size_t len = std::strftime(buffer, sizeof(buffer), pattern.c_str(), &tm);
    if (len == 0)
    {
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);
    }
    return buffer;
}

} // namespace

LocalDateTime::LocalDateTime(LocalDate date,
                             unsigned hour,
                             unsigned minute,
                             unsigned second,
                             unsigned millisecond) noexcept
    : mDate{date}, mHour{hour}, mMinute{minute}, mSecond{second}, mMillisecond{millisecond}
{
}

LocalDateTime LocalDateTime::now() noexcept
{
    std::time_t sec = 0;
    unsigned ms = 0;
    splitNowMs(sec, ms);
    return fromTmAndMs(toLocalTm(sec), ms);
}

LocalDateTime LocalDateTime::nowUTC() noexcept
{
    std::time_t sec = 0;
    unsigned ms = 0;
    splitNowMs(sec, ms);
    return fromTmAndMs(toUTCTm(sec), ms);
}

std::optional<LocalDateTime> LocalDateTime::parse(std::string_view text, std::string_view pattern)
{
    if (text.empty() || pattern.empty())
    {
        return std::nullopt;
    }

    // Pull out %f if present so std::get_time doesn't see it.
    std::string corePattern;
    bool patternHasMs = false;
    for (std::size_t i = 0; i < pattern.size(); ++i)
    {
        if (i + 1 < pattern.size() && pattern[i] == '%' && pattern[i + 1] == 'f')
        {
            patternHasMs = true;
            ++i;
            continue;
        }
        corePattern.push_back(pattern[i]);
    }

    std::tm tm{};
    std::istringstream iss{std::string{text}};
    iss >> std::get_time(&tm, corePattern.c_str());
    if (iss.fail())
    {
        return std::nullopt;
    }

    unsigned ms = 0;
    if (patternHasMs)
    {
        // Read up to 3 trailing digits as milliseconds. Tolerant on whitespace.
        while (iss && std::isspace(iss.peek()))
        {
            iss.get();
        }
        std::string digits;
        while (iss && std::isdigit(iss.peek()) && digits.size() < 3)
        {
            digits.push_back(static_cast<char>(iss.get()));
        }
        if (!digits.empty())
        {
            while (digits.size() < 3)
            {
                digits.push_back('0');
            }
            int v = 0;
            std::from_chars(digits.data(), digits.data() + digits.size(), v);
            ms = static_cast<unsigned>(v);
        }
    }

    LocalDate date{tm.tm_year + 1900, static_cast<unsigned>(tm.tm_mon + 1), static_cast<unsigned>(tm.tm_mday)};
    if (!date.isValid())
    {
        return std::nullopt;
    }
    return LocalDateTime{date,
                         static_cast<unsigned>(tm.tm_hour),
                         static_cast<unsigned>(tm.tm_min),
                         static_cast<unsigned>(tm.tm_sec),
                         ms};
}

LocalDate LocalDateTime::date() const noexcept
{
    return mDate;
}

unsigned LocalDateTime::hour() const noexcept
{
    return mHour;
}

unsigned LocalDateTime::minute() const noexcept
{
    return mMinute;
}

unsigned LocalDateTime::second() const noexcept
{
    return mSecond;
}

unsigned LocalDateTime::millisecond() const noexcept
{
    return mMillisecond;
}

std::string LocalDateTime::format(std::string_view pattern) const
{
    if (pattern.empty())
    {
        return {};
    }

    // Replace %f with three-digit millisecond before strftime.
    std::string expanded;
    expanded.reserve(pattern.size() + 4);
    for (std::size_t i = 0; i < pattern.size(); ++i)
    {
        if (i + 1 < pattern.size() && pattern[i] == '%' && pattern[i + 1] == 'f')
        {
            char msBuf[8]{};
            std::snprintf(msBuf, sizeof(msBuf), "%03u", mMillisecond);
            expanded.append(msBuf);
            ++i;
            continue;
        }
        expanded.push_back(pattern[i]);
    }

    std::tm tm{};
    tm.tm_year = mDate.year() - 1900;
    tm.tm_mon = static_cast<int>(mDate.month()) - 1;
    tm.tm_mday = static_cast<int>(mDate.day());
    tm.tm_hour = static_cast<int>(mHour);
    tm.tm_min = static_cast<int>(mMinute);
    tm.tm_sec = static_cast<int>(mSecond);
    if (mDate.isValid())
    {
        tm.tm_wday = static_cast<int>(mDate.weekday() % 7);   // Sun=0
        tm.tm_yday = static_cast<int>(mDate.dayOfYear()) - 1; // 0-based
    }
    return strftimeToString(tm, expanded);
}

std::strong_ordering operator<=>(const LocalDateTime& lhs, const LocalDateTime& rhs) noexcept
{
    if (auto c = lhs.mDate <=> rhs.mDate; c != std::strong_ordering::equal)
    {
        return c;
    }
    if (auto c = lhs.mHour <=> rhs.mHour; c != std::strong_ordering::equal)
    {
        return c;
    }
    if (auto c = lhs.mMinute <=> rhs.mMinute; c != std::strong_ordering::equal)
    {
        return c;
    }
    if (auto c = lhs.mSecond <=> rhs.mSecond; c != std::strong_ordering::equal)
    {
        return c;
    }
    return lhs.mMillisecond <=> rhs.mMillisecond;
}

bool operator==(const LocalDateTime& lhs, const LocalDateTime& rhs) noexcept
{
    return (lhs <=> rhs) == std::strong_ordering::equal;
}

} // namespace ucf::utilities
