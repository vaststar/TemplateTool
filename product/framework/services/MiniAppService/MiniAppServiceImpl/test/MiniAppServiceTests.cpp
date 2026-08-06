#include <catch2/catch_test_macros.hpp>

#include <fakes/ucf/Services/FakeMiniAppService.h>

using namespace ucf::service;

TEST_CASE("FakeMiniAppService can be constructed", "[MiniAppService]")
{
    auto fakeService = std::make_shared<fakes::FakeMiniAppService>();
    REQUIRE(fakeService != nullptr);
}
