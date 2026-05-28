#pragma once

#include <memory>
#include <string>
#include <vector>

#include <ucf/Utilities/NotificationHelper/INotificationHelper.h>
#include <ucf/Services/ServiceDeclaration/IService.h>
#include <ucf/Services/CameraDirectoryService/ICameraDirectoryServiceCallback.h>

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service {

class SERVICE_EXPORT ICameraDirectoryService
    : public IService
    , public virtual ucf::utilities::INotificationHelper<ICameraDirectoryServiceCallback>
{
public:
    ICameraDirectoryService() = default;
    ICameraDirectoryService(const ICameraDirectoryService&) = delete;
    ICameraDirectoryService(ICameraDirectoryService&&) = delete;
    ICameraDirectoryService& operator=(const ICameraDirectoryService&) = delete;
    ICameraDirectoryService& operator=(ICameraDirectoryService&&) = delete;
    virtual ~ICameraDirectoryService() = default;

public:
    // ===== Read =====
    [[nodiscard]] virtual model::CameraGroupArray  getCameraGroups()  const = 0;
    [[nodiscard]] virtual model::CameraEntryArray  getCameras()       const = 0;
    [[nodiscard]] virtual model::CameraDirectoryRelationArray getCameraRelations() const = 0;

    [[nodiscard]] virtual model::ICameraGroupPtr getCameraGroup(const std::string& nodeId) const = 0;
    [[nodiscard]] virtual model::ICameraEntryPtr getCamera(const std::string& nodeId) const = 0;

    // ===== Batch write: caller supplies arrays of interface implementations (id 由 caller 生成) =====
    virtual void addCameraGroups(const model::CameraGroupArray& groups) = 0;
    virtual void updateCameraGroups(const model::CameraGroupArray& groups) = 0;
    virtual void removeCameraGroups(const std::vector<std::string>& nodeIds) = 0;

    virtual void addCameras(const model::CameraEntryArray& cameras) = 0;
    virtual void updateCameras(const model::CameraEntryArray& cameras) = 0;
    virtual void removeCameras(const std::vector<std::string>& nodeIds) = 0;

    virtual void addCameraRelations(const model::CameraDirectoryRelationArray& relations) = 0;
    virtual void updateCameraRelations(const model::CameraDirectoryRelationArray& relations) = 0;
    virtual void removeCameraRelations(const std::vector<std::string>& childIds) = 0;

public:
    static std::shared_ptr<ICameraDirectoryService> createInstance(ucf::framework::ICoreFrameworkWPtr coreFramework);
};

} // namespace ucf::service
