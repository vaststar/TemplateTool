#pragma once

#include <trompeloeil.hpp>
#include <ucf/Services/CameraDirectoryService/ICameraDirectoryService.h>
#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>

namespace ucf::service::fakes {

/**
 * @brief Fake implementation of ICameraDirectoryService for unit testing
 *
 * Usage:
 *   auto fakeService = std::make_shared<FakeCameraDirectoryService>();
 *   ALLOW_CALL(*fakeService, isCameraDirectoryReady()).RETURN(true);
 */
class FakeCameraDirectoryService
    : public ICameraDirectoryService
    , public ucf::utilities::NotificationHelper<ICameraDirectoryServiceCallback>
{
public:
    FakeCameraDirectoryService() = default;
    ~FakeCameraDirectoryService() override = default;

    // IService
    MAKE_CONST_MOCK0(getServiceName, std::string(), override);

    // ICameraDirectoryService - Read
    MAKE_CONST_MOCK0(getCameraGroups, model::CameraGroupArray(), override);
    MAKE_CONST_MOCK0(getCameras, model::CameraEntryArray(), override);
    MAKE_CONST_MOCK0(getCameraRelations, model::CameraDirectoryRelationArray(), override);
    MAKE_CONST_MOCK1(getCameraGroup, model::ICameraGroupPtr(const std::string&), override);
    MAKE_CONST_MOCK1(getCamera, model::ICameraEntryPtr(const std::string&), override);

    // ICameraDirectoryService - Batch write (groups)
    MAKE_MOCK1(addCameraGroups, void(const model::CameraGroupArray&), override);
    MAKE_MOCK1(updateCameraGroups, void(const model::CameraGroupArray&), override);
    MAKE_MOCK1(removeCameraGroups, void(const std::vector<std::string>&), override);

    // ICameraDirectoryService - Batch write (cameras)
    MAKE_MOCK1(addCameras, void(const model::CameraEntryArray&), override);
    MAKE_MOCK1(updateCameras, void(const model::CameraEntryArray&), override);
    MAKE_MOCK1(removeCameras, void(const std::vector<std::string>&), override);

    // ICameraDirectoryService - Batch write (relations)
    MAKE_MOCK1(addCameraRelations, void(const model::CameraDirectoryRelationArray&), override);
    MAKE_MOCK1(updateCameraRelations, void(const model::CameraDirectoryRelationArray&), override);
    MAKE_MOCK1(removeCameraRelations, void(const std::vector<std::string>&), override);

    // ICameraDirectoryService - Lifecycle
    MAKE_MOCK0(loadCameraDirectory, void(), override);
    MAKE_CONST_MOCK0(isCameraDirectoryReady, bool(), override);

protected:
    void initService() override {}
};

} // namespace ucf::service::fakes
