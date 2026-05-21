#include <ucf/Utilities/TimeUtils/Instant.h>

#include <ucf/Utilities/TimeUtils/LocalDateTime.h>

#include <charconv>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <ctime>
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

std::time_t timegmPortable(std::tm& tm)
{
#if defined(_WIN32)
    return _mkgmtime(&tm);
#else
    return timegm(&tm);
#endif
}

} // namespace

Instant::Instant(TimePoint tp) noexcept
    : mTp{tp}
{
}

Instant Instant::now() noexcept
{
    return Instant{std::chrono::time_point_cast<std::chrono::milliseconds>(Clock::now())};
}

Instant Instant::fromUnixMilliseconds(int64_t ms) noexcept
{
    return Instant{TimePoint{std::chrono::milliseconds{ms}}};
}

Instant Instant::fromUnixSeconds(int64_t s) noexcept
{
    return Instant{TimePoint{std::chrono::milliseconds{s * 1000}}};
}

std::optional<Instant> Instant::parseISO8601(std::string_view text)
{
    // Accepted forms:
    //   2026-05-21T10:30:00Z
    //   2026-05-21T10:30:00.123Z
    if (text.size() < 20 || text.back() != 'Z')
    {
        return std::nullopt;
    }
    if (text[4] != '-' || text[7] != '-' || text[10] != 'T'
        || text[13] != ':' || text[16] != ':')
    {
        return std::nullopt;
    }

    auto parseField = [](std::string_view s, auto& out) -> bool
    {
        const char* finish = s.data() + s.size();
        auto [ptr, ec] = std::from_chars(s.data(), finish, out);
        return ec == std::errc{} && ptr == finish;
    };

    int y = 0;
    unsigned mo = 0;
    unsigned d = 0;
    unsigned h = 0;
    unsigned mi = 0;
    unsigned s = 0;
    if (!parseField(text.substr(0, 4), y)
        || !parseField(text.substr(5, 2), mo)
        || !parseField(text.substr(8, 2), d)
        || !parseField(text.substr(11, 2), h)
        || !parseField(text.substr(14, 2), mi)
        || !parseField(text.substr(17, 2), s))
    {
        return std::nullopt;
    }

    unsigned ms = 0;
    if (text.size() > 20)
    {
        // Must look like ".fff" before 'Z'.
        if (text[19] != '.')
        {
            return std::nullopt;
        }
        auto frac = text.substr(20, text.size() - 21);
        if (frac.empty() || frac.size() > 3)
        {
            return std::nullopt;
        }
        std::string padded{frac};
        while (padded.size() < 3)
        {
            padded.push_back('0');
        }
        if (!parseField(padded, ms))
        {
            return std::nullopt;
        }
    }

    std::tm tm{};
    tm.tm_year = y - 1900;
    tm.tm_mon = static_cast<int>(mo) - 1;
    tm.tm_mday = static_cast<int>(d);
    tm.tm_hour = static_cast<int>(h);
    tm.tm_min = static_cast<int>(mi);
    tm.tm_sec = static_cast<int>(s);
    std::time_t t = timegmPortable(tm);
    if (t == static_cast<std::time_t>(-1))
    {
        return std::nullopt;
    }
    auto tp = TimePoint{std::chrono::milliseconds{static_cast<int64_t>(t) * 1000 + ms}};
    return Instant{tp};
}

Instant::TimePoint Instant::timePoint() const noexcept
{
    return mTp;
}

int64_t Instant::toUnixMilliseconds() const noexcept
{
    return mTp.time_since_epoch().count();
}

int64_t Instant::toUnixSeconds() const noexcept
{
    return std::chrono::duration_cast<std::chrono::seconds>(mTp.time_since_epoch()).count();
}

std::string Instant::toISO8601() const
{
    auto secs = std::chrono::time_point_cast<std::chrono::seconds>(mTp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(mTp - secs).count();
    std::time_t t = static_cast<std::time_t>(secs.time_since_epoch().count());
    std::tm tm = toUTCTm(t);
    char buf[40]{};
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:%02d.%03lldZ",
                  tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                  tm.tm_hour, tm.tm_min, tm.tm_sec,
                  static_cast<long long>(ms));
    return buf;
}

LocalDateTime Instant::toUTCDateTime() const
{
    auto secs = std::chrono::time_point_cast<std::chrono::seconds>(mTp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(mTp - secs).count();
    std::time_t t = static_cast<std::time_t>(secs.time_since_epoch().count());
    return fromTmAndMs(toUTCTm(t), static_cast<unsigned>(ms));
}

LocalDateTime Instant::toLocalDateTime() const
{
    auto secs = std::chrono::time_point_cast<std::chrono::seconds>(mTp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(mTp - secs).count();
    std::time_t t = static_cast<std::time_t>(secs.time_since_epoch().count());
    return fromTmAndMs(toLocalTm(t), static_cast<unsigned>(ms));
}

Instant& Instant::operator+=(std::chrono::milliseconds d) noexcept
{
    mTp += d;
    return *this;
}

Instant& Instant::operator-=(std::chrono::milliseconds d) noexcept
{
    mTp -= d;
    return *this;
}

std::strong_ordering operator<=>(const Instant& lhs, const Instant& rhs) noexcept
{
    return lhs.mTp <=> rhs.mTp;
}

bool operator==(const Instant& lhs, const Instant& rhs) noexcept
{
    return lhs.mTp == rhs.mTp;
}

Instant operator+(Instant lhs, std::chrono::milliseconds rhs) noexcept
{
    lhs += rhs;
    return lhs;
}

Instant operator-(Instant lhs, std::chrono::milliseconds rhs) noexcept
{
    lhs -= rhs;
    return lhs;
}

std::chrono::milliseconds operator-(const Instant& lhs, const Instant& rhs) noexcept
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(lhs.mTp - rhs.mTp);
}

} // namespace ucf::utilities
