#include <catch2/catch_test_macros.hpp>

#include <fakes/ucf/Services/FakeCameraDirectoryService.h>

using namespace ucf::service;

TEST_CASE("FakeCameraDirectoryService can be constructed", "[CameraDirectoryService]")
{
    auto fakeService = std::make_shared<fakes::FakeCameraDirectoryService>();
    REQUIRE(fakeService != nullptr);
}
