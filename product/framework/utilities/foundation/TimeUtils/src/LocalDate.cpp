#include <ucf/Utilities/TimeUtils/LocalDate.h>

#include <array>
#include <charconv>
#include <cstdio>
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

LocalDate fromTm(const std::tm& tm) noexcept
{
    return LocalDate{tm.tm_year + 1900, static_cast<unsigned>(tm.tm_mon + 1), static_cast<unsigned>(tm.tm_mday)};
}

} // namespace

LocalDate::LocalDate(int year, unsigned month, unsigned day) noexcept
    : mYmd{std::chrono::year{year}, std::chrono::month{month}, std::chrono::day{day}}
{
}

LocalDate LocalDate::today() noexcept
{
    auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    return fromTm(toLocalTm(t));
}

LocalDate LocalDate::todayUTC() noexcept
{
    auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    return fromTm(toUTCTm(t));
}

std::optional<LocalDate> LocalDate::parse(std::string_view text)
{
    // Strict "YYYY-MM-DD".
    if (text.size() != 10 || text[4] != '-' || text[7] != '-')
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
    unsigned m = 0;
    unsigned d = 0;
    if (!parseField(text.substr(0, 4), y)
        || !parseField(text.substr(5, 2), m)
        || !parseField(text.substr(8, 2), d))
    {
        return std::nullopt;
    }

    LocalDate result{y, m, d};
    if (!result.isValid())
    {
        return std::nullopt;
    }
    return result;
}

bool LocalDate::isValid() const noexcept
{
    return mYmd.ok();
}

int LocalDate::year() const noexcept
{
    return static_cast<int>(mYmd.year());
}

unsigned LocalDate::month() const noexcept
{
    return static_cast<unsigned>(mYmd.month());
}

unsigned LocalDate::day() const noexcept
{
    return static_cast<unsigned>(mYmd.day());
}

unsigned LocalDate::dayOfYear() const noexcept
{
    if (!mYmd.ok())
    {
        return 0;
    }
    auto jan1 = std::chrono::sys_days{std::chrono::year{static_cast<int>(mYmd.year())} / 1 / 1};
    auto self = std::chrono::sys_days{mYmd};
    return static_cast<unsigned>((self - jan1).count() + 1);
}

unsigned LocalDate::weekday() const noexcept
{
    if (!mYmd.ok())
    {
        return 0;
    }
    std::chrono::weekday wd{std::chrono::sys_days{mYmd}};
    return wd.iso_encoding();
}

std::string LocalDate::toString() const
{
    char buf[16]{};
    std::snprintf(buf, sizeof(buf), "%04d-%02u-%02u", year(), month(), day());
    return buf;
}

LocalDate& LocalDate::operator+=(std::chrono::days d) noexcept
{
    mYmd = std::chrono::year_month_day{std::chrono::sys_days{mYmd} + d};
    return *this;
}

LocalDate& LocalDate::operator-=(std::chrono::days d) noexcept
{
    mYmd = std::chrono::year_month_day{std::chrono::sys_days{mYmd} - d};
    return *this;
}

std::strong_ordering operator<=>(const LocalDate& lhs, const LocalDate& rhs) noexcept
{
    return std::chrono::sys_days{lhs.mYmd} <=> std::chrono::sys_days{rhs.mYmd};
}

bool operator==(const LocalDate& lhs, const LocalDate& rhs) noexcept
{
    return std::chrono::sys_days{lhs.mYmd} == std::chrono::sys_days{rhs.mYmd};
}

LocalDate operator+(LocalDate lhs, std::chrono::days rhs) noexcept
{
    lhs += rhs;
    return lhs;
}

LocalDate operator-(LocalDate lhs, std::chrono::days rhs) noexcept
{
    lhs -= rhs;
    return lhs;
}

std::chrono::days operator-(const LocalDate& lhs, const LocalDate& rhs) noexcept
{
    return std::chrono::sys_days{lhs.mYmd} - std::chrono::sys_days{rhs.mYmd};
}

} // namespace ucf::utilities
