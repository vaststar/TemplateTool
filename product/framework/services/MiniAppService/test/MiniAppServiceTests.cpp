#include <catch2/catch_test_macros.hpp>

#include <fakes/ucf/CoreFramework/FakeCoreFramework.h>
#include <ucf/services/MiniAppService/MiniAppServiceCreator.h>
#include <ucf/services/MiniAppService/IMiniAppService.h>

TEST_CASE("MiniAppService can be created through its public creator", "[MiniAppService]")
{
    auto fakeCoreFramework = std::make_shared<ucf::framework::fakes::FakeCoreFramework>();
    REQUIRE(fakeCoreFramework != nullptr);

    auto service = ucf::service::impl::createMiniAppService(fakeCoreFramework);
    REQUIRE(service != nullptr);
    REQUIRE(service->getServiceName() == "MiniAppService");
}
