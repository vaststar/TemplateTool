#include <catch2/catch_test_macros.hpp>

#include <fakes/ucf/CoreFramework/FakeCoreFramework.h>
#include "MiniAppService.h"

TEST_CASE("MiniAppService can be constructed", "[MiniAppService]")
{
    auto fakeCoreFramework = std::make_shared<ucf::framework::fakes::FakeCoreFramework>();
    REQUIRE(fakeCoreFramework != nullptr);

    auto service = std::make_shared<ucf::service::MiniAppService>(fakeCoreFramework);
    REQUIRE(service != nullptr);
}
