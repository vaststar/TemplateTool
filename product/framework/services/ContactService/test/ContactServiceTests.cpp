#include <catch2/catch_test_macros.hpp>

#include <fakes/ucf/CoreFramework/FakeCoreFramework.h>
#include "ContactService.h"

TEST_CASE("ContactService can be constructed", "[ContactService]")
{
    auto fakeCoreFramework = std::make_shared<ucf::framework::fakes::FakeCoreFramework>();
    REQUIRE(fakeCoreFramework != nullptr);

    auto service = std::make_shared<ucf::service::ContactService>(fakeCoreFramework);
    REQUIRE(service != nullptr);
}
