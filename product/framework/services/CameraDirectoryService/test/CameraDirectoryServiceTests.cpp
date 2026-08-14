#include <catch2/catch_test_macros.hpp>

#include <fakes/ucf/CoreFramework/FakeCoreFramework.h>
#include <ucf/services/CameraDirectoryService/CameraDirectoryServiceCreator.h>
#include <ucf/services/CameraDirectoryService/ICameraDirectoryService.h>

TEST_CASE("CameraDirectoryService can be created through its public creator", "[CameraDirectoryService]")
{
    auto fakeCoreFramework = std::make_shared<ucf::framework::fakes::FakeCoreFramework>();
    REQUIRE(fakeCoreFramework != nullptr);

    auto service = ucf::service::impl::createCameraDirectoryService(fakeCoreFramework);
    REQUIRE(service != nullptr);
    REQUIRE(service->getServiceName() == "CameraDirectoryService");
}
