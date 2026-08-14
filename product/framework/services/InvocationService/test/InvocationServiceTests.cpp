#include <catch2/catch_test_macros.hpp>

#include <fakes/ucf/CoreFramework/FakeCoreFramework.h>
#include <ucf/services/InvocationService/InvocationServiceCreator.h>
#include <ucf/services/InvocationService/IInvocationService.h>

TEST_CASE("InvocationService can be created through its public creator", "[InvocationService]")
{
    auto fakeCoreFramework = std::make_shared<ucf::framework::fakes::FakeCoreFramework>();
    REQUIRE(fakeCoreFramework != nullptr);

    auto service = ucf::service::impl::createInvocationService(fakeCoreFramework);
    REQUIRE(service != nullptr);
    REQUIRE(service->getServiceName() == "InvocationService");
}
