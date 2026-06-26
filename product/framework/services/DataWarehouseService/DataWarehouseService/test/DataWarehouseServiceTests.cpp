#include <catch2/catch_test_macros.hpp>

#include <fakes/ucf/Services/FakeDataWarehouseService.h>

using namespace ucf::service;

TEST_CASE("FakeDataWarehouseService can be constructed", "[DataWarehouseService]")
{
    auto fakeService = std::make_shared<fakes::FakeDataWarehouseService>();
    REQUIRE(fakeService != nullptr);
}
