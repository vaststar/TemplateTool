#include <catch2/catch_test_macros.hpp>

#include <fakes/ucf/Services/FakeMediaService.h>

using namespace ucf::service;

TEST_CASE("FakeMediaService can be constructed", "[MediaService]")
{
    auto fakeService = std::make_shared<fakes::FakeMediaService>();
    REQUIRE(fakeService != nullptr);
}
