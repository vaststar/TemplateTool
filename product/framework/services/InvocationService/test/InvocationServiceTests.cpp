#include <catch2/catch_test_macros.hpp>

#include <fakes/ucf/CoreFramework/FakeCoreFramework.h>
#include "InvocationService.h"

TEST_CASE("InvocationService can be constructed", "[InvocationService]")
{
    auto fakeCoreFramework = std::make_shared<ucf::framework::fakes::FakeCoreFramework>();
    REQUIRE(fakeCoreFramework != nullptr);

    auto service = std::make_shared<ucf::service::impl::InvocationService>(fakeCoreFramework);
    REQUIRE(service != nullptr);
}
