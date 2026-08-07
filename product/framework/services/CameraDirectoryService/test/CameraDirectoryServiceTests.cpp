#include <catch2/catch_test_macros.hpp>

#include <fakes/ucf/CoreFramework/FakeCoreFramework.h>
#include "CameraDirectoryService.h"

TEST_CASE("CameraDirectoryService can be constructed", "[CameraDirectoryService]")
{
    auto fakeCoreFramework = std::make_shared<ucf::framework::fakes::FakeCoreFramework>();
    REQUIRE(fakeCoreFramework != nullptr);

    auto service = std::make_shared<ucf::service::CameraDirectoryService>(fakeCoreFramework);
    REQUIRE(service != nullptr);
}
