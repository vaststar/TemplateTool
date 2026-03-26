#include <catch2/catch_test_macros.hpp>

#include <fakes/ucf/CoreFramework/FakeCoreFramework.h>
#include "ClientInfoService.h"

TEST_CASE( "Basic Test for CoreFramework", "[CoreFrameworkTests]" )
{
    auto fakeCoreFramework = std::make_shared<ucf::framework::fakes::FakeCoreFramework>();
    REQUIRE(fakeCoreFramework != nullptr);

    auto clientInfoService = std::make_shared<ucf::service::ClientInfoService>(fakeCoreFramework);
    REQUIRE(clientInfoService != nullptr);
}