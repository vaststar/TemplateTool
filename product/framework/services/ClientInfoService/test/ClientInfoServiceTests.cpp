#include <catch2/catch_test_macros.hpp>

#include <fakes/ucf/CoreFramework/FakeCoreFramework.h>
#include <ucf/services/ClientInfoService/ClientInfoServiceCreator.h>
#include <ucf/services/ClientInfoService/IClientInfoService.h>

TEST_CASE("ClientInfoService can be created through its public creator", "[ClientInfoService]")
{
    auto fakeCoreFramework = std::make_shared<ucf::framework::fakes::FakeCoreFramework>();
    REQUIRE(fakeCoreFramework != nullptr);

    auto clientInfoService = ucf::service::impl::createClientInfoService(fakeCoreFramework);
    REQUIRE(clientInfoService != nullptr);
    REQUIRE(clientInfoService->getServiceName() == "ClientInfoService");
}
