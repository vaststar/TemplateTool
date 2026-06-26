#include <catch2/catch_test_macros.hpp>

#include <fakes/ucf/Services/FakeNetworkService.h>

using namespace ucf::service;

TEST_CASE("FakeNetworkService can be constructed", "[NetworkService]")
{
    auto fakeService = std::make_shared<fakes::FakeNetworkService>();
    REQUIRE(fakeService != nullptr);
}
