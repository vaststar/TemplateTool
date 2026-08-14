#include <catch2/catch_test_macros.hpp>

#include <fakes/ucf/CoreFramework/FakeCoreFramework.h>
#include <ucf/services/FeatureSettingsService/FeatureSettingsServiceCreator.h>
#include <ucf/services/FeatureSettingsService/IFeatureSettingsService.h>

TEST_CASE("FeatureSettingsService can be created through its public creator", "[FeatureSettingsService]")
{
    auto fakeCoreFramework = std::make_shared<ucf::framework::fakes::FakeCoreFramework>();
    REQUIRE(fakeCoreFramework != nullptr);

    auto service = ucf::service::impl::createFeatureSettingsService(fakeCoreFramework);
    REQUIRE(service != nullptr);
    REQUIRE(service->getServiceName() == "FeatureSettingsService");
}
