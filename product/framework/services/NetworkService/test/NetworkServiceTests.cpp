#include <catch2/catch_test_macros.hpp>

#include <fakes/ucf/CoreFramework/FakeCoreFramework.h>
#include "NetworkService.h"

TEST_CASE("NetworkService can be constructed", "[NetworkService]")
{
    auto fakeCoreFramework = std::make_shared<ucf::framework::fakes::FakeCoreFramework>();
    REQUIRE(fakeCoreFramework != nullptr);

    auto service = std::make_shared<ucf::service::NetworkService>(fakeCoreFramework);
    REQUIRE(service != nullptr);
}
