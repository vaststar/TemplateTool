#include <catch2/catch_test_macros.hpp>

#include <fakes/ucf/CoreFramework/FakeCoreFramework.h>
#include "ServiceFactory.h"

TEST_CASE("ServiceFactory can be constructed", "[ServiceFactory]")
{
    auto fakeCoreFramework = std::make_shared<ucf::framework::fakes::FakeCoreFramework>();
    REQUIRE(fakeCoreFramework != nullptr);

    auto factory = std::make_shared<ucf::service::ServiceFactory>(fakeCoreFramework);
    REQUIRE(factory != nullptr);
}
