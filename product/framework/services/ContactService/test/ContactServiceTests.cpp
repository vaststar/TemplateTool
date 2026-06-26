#include <catch2/catch_test_macros.hpp>

#include <fakes/ucf/Services/FakeContactService.h>

using namespace ucf::service;

TEST_CASE("FakeContactService can be constructed", "[ContactService]")
{
    auto fakeService = std::make_shared<fakes::FakeContactService>();
    REQUIRE(fakeService != nullptr);
}
