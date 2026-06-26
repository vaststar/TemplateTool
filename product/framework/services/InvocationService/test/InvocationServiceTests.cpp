#include <catch2/catch_test_macros.hpp>

#include <fakes/ucf/Services/FakeInvocationService.h>

using namespace ucf::service;

TEST_CASE("FakeInvocationService can be constructed", "[InvocationService]")
{
    auto fakeService = std::make_shared<fakes::FakeInvocationService>();
    REQUIRE(fakeService != nullptr);
}
