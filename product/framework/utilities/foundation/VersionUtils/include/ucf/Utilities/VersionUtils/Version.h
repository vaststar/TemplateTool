#pragma once

#include <compare>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <ucf/Utilities/VersionUtils/VersionUtilsExport.h>

namespace ucf::utilities {

// Dot-separated numeric version, e.g. "2026.04.0.1523".
// Pure value type, no service-layer dependency.
class VERSION_UTILS_API Version final
{
public:
    using Segment = std::uint32_t;

    Version() = default;
    explicit Version(std::vector<Segment> segments);

    // Returns nullopt on empty input, empty segments, non-numeric, or overflow.
    [[nodiscard]] static std::optional<Version> parse(std::string_view text);

    // Returns false if either input fails to parse.
    [[nodiscard]] static bool isNewer(std::string_view candidate, std::string_view baseline);

    [[nodiscard]] const std::vector<Segment>& segments() const noexcept;
    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] std::string toString() const;

    // Pads shorter operand with zero segments before comparing.
    [[nodiscard]] std::strong_ordering compare(const Version& other) const noexcept;

    friend VERSION_UTILS_API std::strong_ordering operator<=>(const Version& lhs, const Version& rhs) noexcept;
    friend VERSION_UTILS_API bool operator==(const Version& lhs, const Version& rhs) noexcept;

private:
    std::vector<Segment> mSegments;
};

} // namespace ucf::utilities
