#include <catch2/catch_test_macros.hpp>

#include <fakes/ucf/CoreFramework/FakeCoreFramework.h>
#include "DataWarehouseService.h"

TEST_CASE("DataWarehouseService can be constructed", "[DataWarehouseService]")
{
    auto fakeCoreFramework = std::make_shared<ucf::framework::fakes::FakeCoreFramework>();
    REQUIRE(fakeCoreFramework != nullptr);

    auto service = std::make_shared<ucf::service::impl::DataWarehouseService>(fakeCoreFramework);
    REQUIRE(service != nullptr);
}
