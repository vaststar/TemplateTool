#include <catch2/catch_test_macros.hpp>

#include <fakes/ucf/Services/FakeFeatureSettingsService.h>

using namespace ucf::service;

TEST_CASE("FakeFeatureSettingsService can be constructed", "[FeatureSettingsService]")
{
    auto fakeService = std::make_shared<fakes::FakeFeatureSettingsService>();
    REQUIRE(fakeService != nullptr);
}
