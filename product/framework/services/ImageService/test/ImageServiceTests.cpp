#include <catch2/catch_test_macros.hpp>

#include <fakes/ucf/Services/FakeImageService.h>

using namespace ucf::service;

TEST_CASE("FakeImageService can be constructed", "[ImageService]")
{
    auto fakeService = std::make_shared<fakes::FakeImageService>();
    REQUIRE(fakeService != nullptr);
}
