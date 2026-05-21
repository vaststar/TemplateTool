#include <ucf/Utilities/VersionUtils/Version.h>

#include <algorithm>
#include <charconv>
#include <sstream>
#include <system_error>

namespace ucf::utilities {

Version::Version(std::vector<Segment> segments)
    : mSegments(std::move(segments))
{
}

std::optional<Version> Version::parse(std::string_view text)
{
    if (text.empty())
    {
        return std::nullopt;
    }

    auto parseSegment = [](std::string_view s) -> std::optional<Segment>
    {
        if (s.empty())
        {
            return std::nullopt;
        }
        Segment value = 0;
        const char* finish = s.data() + s.size();
        auto [ptr, ec] = std::from_chars(s.data(), finish, value);
        if (ec != std::errc{} || ptr != finish)
        {
            return std::nullopt;
        }
        return value;
    };

    std::vector<Segment> segments;
    while (!text.empty())
    {
        auto dot = text.find('.');
        auto seg = (dot == std::string_view::npos) ? text : text.substr(0, dot);
        auto value = parseSegment(seg);
        if (!value)
        {
            return std::nullopt;
        }
        segments.push_back(*value);

        if (dot == std::string_view::npos)
        {
            break;
        }
        text.remove_prefix(dot + 1);
        if (text.empty())
        {
            // Trailing dot.
            return std::nullopt;
        }
    }

    return Version{std::move(segments)};
}

bool Version::isNewer(std::string_view candidate, std::string_view baseline)
{
    auto lhs = parse(candidate);
    auto rhs = parse(baseline);
    if (!lhs || !rhs)
    {
        return false;
    }
    return lhs->compare(*rhs) == std::strong_ordering::greater;
}

const std::vector<Version::Segment>& Version::segments() const noexcept
{
    return mSegments;
}

bool Version::empty() const noexcept
{
    return mSegments.empty();
}

std::string Version::toString() const
{
    if (mSegments.empty())
    {
        return {};
    }
    std::ostringstream oss;
    for (std::size_t i = 0; i < mSegments.size(); ++i)
    {
        if (i != 0)
        {
            oss << '.';
        }
        oss << mSegments[i];
    }
    return oss.str();
}

std::strong_ordering Version::compare(const Version& other) const noexcept
{
    const std::size_t maxLen = std::max(mSegments.size(), other.mSegments.size());
    for (std::size_t i = 0; i < maxLen; ++i)
    {
        Segment a = (i < mSegments.size()) ? mSegments[i] : 0;
        Segment b = (i < other.mSegments.size()) ? other.mSegments[i] : 0;
        if (auto cmp = a <=> b; cmp != std::strong_ordering::equal)
        {
            return cmp;
        }
    }
    return std::strong_ordering::equal;
}

std::strong_ordering operator<=>(const Version& lhs, const Version& rhs) noexcept
{
    return lhs.compare(rhs);
}

bool operator==(const Version& lhs, const Version& rhs) noexcept
{
    return lhs.compare(rhs) == std::strong_ordering::equal;
}

} // namespace ucf::utilities
