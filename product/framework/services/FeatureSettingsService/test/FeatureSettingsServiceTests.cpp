#include <catch2/catch_test_macros.hpp>

#include <fakes/ucf/CoreFramework/FakeCoreFramework.h>
#include "FeatureSettingsService.h"

TEST_CASE("FeatureSettingsService can be constructed", "[FeatureSettingsService]")
{
    auto fakeCoreFramework = std::make_shared<ucf::framework::fakes::FakeCoreFramework>();
    REQUIRE(fakeCoreFramework != nullptr);

    auto service = std::make_shared<ucf::service::FeatureSettingsService>(fakeCoreFramework);
    REQUIRE(service != nullptr);
}
