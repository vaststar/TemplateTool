#include <catch2/catch_test_macros.hpp>

#include <fakes/ucf/CoreFramework/FakeCoreFramework.h>
#include "ImageService.h"

TEST_CASE("ImageService can be constructed", "[ImageService]")
{
    auto fakeCoreFramework = std::make_shared<ucf::framework::fakes::FakeCoreFramework>();
    REQUIRE(fakeCoreFramework != nullptr);

    auto service = std::make_shared<ucf::service::ImageService>(fakeCoreFramework);
    REQUIRE(service != nullptr);
}
