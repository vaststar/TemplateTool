#include <catch2/catch_test_macros.hpp>

#include <fakes/ucf/CoreFramework/FakeCoreFramework.h>
#include <ucf/services/NetworkService/NetworkServiceCreator.h>
#include <ucf/services/NetworkService/INetworkService.h>

TEST_CASE("NetworkService can be created through its public creator", "[NetworkService]")
{
    auto fakeCoreFramework = std::make_shared<ucf::framework::fakes::FakeCoreFramework>();
    REQUIRE(fakeCoreFramework != nullptr);

    auto service = ucf::service::impl::createNetworkService(fakeCoreFramework);
    REQUIRE(service != nullptr);
    REQUIRE(service->getServiceName() == "NetworkService");
}
