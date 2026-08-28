#include <catch2/catch_test_macros.hpp>

#include <commonhead/viewmodels/ViewModelUtils/TimeDisplayUtils.h>

#include <string>

namespace {

using commonHead::utilities::TimeDisplayFormat;
using commonHead::utilities::TimeDisplayUtils;

} // namespace

TEST_CASE("TimeDisplayUtils formats current user time", "[TimeDisplayUtils]")
{
    const auto formatted = TimeDisplayUtils::formatCurrentUserTime({
        .localPattern = "LOCAL-%Y%m%d-%H%M%S",
        .utcFallbackPattern = "UTC-%Y%m%d-%H%M%SZ",
        .failureText = "failure"
    });

    REQUIRE(formatted != "failure");
    REQUIRE((
        formatted.starts_with("LOCAL-")
        || formatted.starts_with("UTC-")));
}

TEST_CASE("TimeDisplayUtils explicitly falls back to UTC", "[TimeDisplayUtils]")
{
    const auto formatted = TimeDisplayUtils::formatCurrentUserTime({
        .localPattern = "%Q",
        .utcFallbackPattern = "UTC-%Y%m%d-%H%M%SZ",
        .failureText = "failure"
    });

    REQUIRE(formatted.starts_with("UTC-"));
    REQUIRE(formatted.ends_with('Z'));
}

TEST_CASE("TimeDisplayUtils returns caller failure text", "[TimeDisplayUtils]")
{
    const auto formatted = TimeDisplayUtils::formatCurrentUserTime({
        .localPattern = "%Q",
        .utcFallbackPattern = "%Q",
        .failureText = "Time unavailable"
    });

    REQUIRE(formatted == "Time unavailable");
}
