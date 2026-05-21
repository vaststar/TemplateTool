#include <catch2/catch_test_macros.hpp>

#include "ucf/Utilities/VersionUtils/Version.h"

#include <chrono>

using ucf::utilities::Version;

TEST_CASE("Version::parse - valid", "[Version][Parse]")
{
    auto v = Version::parse("2026.04.0.1523");
    REQUIRE(v.has_value());
    REQUIRE(v->segments() == std::vector<Version::Segment>{2026, 4, 0, 1523});
    REQUIRE(v->toString() == "2026.4.0.1523");
}

TEST_CASE("Version::parse - single segment", "[Version][Parse]")
{
    auto v = Version::parse("42");
    REQUIRE(v.has_value());
    REQUIRE(v->segments() == std::vector<Version::Segment>{42});
}

TEST_CASE("Version::parse - rejects malformed", "[Version][Parse]")
{
    REQUIRE_FALSE(Version::parse("").has_value());
    REQUIRE_FALSE(Version::parse(".").has_value());
    REQUIRE_FALSE(Version::parse(".1").has_value());
    REQUIRE_FALSE(Version::parse("1.").has_value());
    REQUIRE_FALSE(Version::parse("1..2").has_value());
    REQUIRE_FALSE(Version::parse("1.abc").has_value());
    REQUIRE_FALSE(Version::parse("-1.0").has_value());
    REQUIRE_FALSE(Version::parse("1.2.99999999999").has_value());
    REQUIRE_FALSE(Version::parse("1 .2").has_value());
}

TEST_CASE("Version - comparison with zero padding", "[Version][Compare]")
{
    auto a = Version::parse("1.2").value();
    auto b = Version::parse("1.2.0").value();
    auto c = Version::parse("1.2.0.0").value();

    REQUIRE(a == b);
    REQUIRE(b == c);
    REQUIRE(a == c);
    REQUIRE_FALSE(a < c);
    REQUIRE_FALSE(c < a);
}

TEST_CASE("Version - comparison ordering", "[Version][Compare]")
{
    auto a = Version::parse("2026.4.0.1523").value();
    auto b = Version::parse("2026.4.0.1524").value();
    auto c = Version::parse("2026.5.0.0").value();

    REQUIRE(a < b);
    REQUIRE(b < c);
    REQUIRE(a < c);
    REQUIRE(c > a);
    REQUIRE_FALSE(a == b);
}

TEST_CASE("Version - comparison across lengths", "[Version][Compare]")
{
    auto shorter = Version::parse("1.3").value();
    auto longer  = Version::parse("1.2.99").value();
    // 1.3 > 1.2.99 because 3 > 2 at segment[1].
    REQUIRE(shorter > longer);
}

TEST_CASE("Version::isNewer", "[Version][IsNewer]")
{
    REQUIRE(Version::isNewer("2.0", "1.9"));
    REQUIRE(Version::isNewer("1.0.1", "1.0.0"));
    REQUIRE_FALSE(Version::isNewer("1.0", "1.0"));
    REQUIRE_FALSE(Version::isNewer("1.0", "1.0.0"));
    REQUIRE_FALSE(Version::isNewer("0.9", "1.0"));
    // Malformed inputs treated as "not newer".
    REQUIRE_FALSE(Version::isNewer("abc", "1.0"));
    REQUIRE_FALSE(Version::isNewer("1.0", "abc"));
}

TEST_CASE("Version - default constructed", "[Version]")
{
    Version v;
    REQUIRE(v.empty());
    REQUIRE(v.toString().empty());

    Version other;
    REQUIRE(v == other);
}
