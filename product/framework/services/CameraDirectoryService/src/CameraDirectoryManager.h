#pragma once

#include <memory>
#include <string>
#include <vector>

#include "CameraDirectoryEntities.h"
#include "CameraDirectoryNotificationSink.h"

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service {

class CameraDirectoryModel;

// Business orchestration layer: currently a thin forwarder onto CameraDirectoryModel.
// Reserved as the place where camera discovery / device hot-plug / default directory
// initialization will live in the future.
class CameraDirectoryManager final
{
public:
    explicit CameraDirectoryManager(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ~CameraDirectoryManager();
    CameraDirectoryManager(const CameraDirectoryManager&) = delete;
    CameraDirectoryManager(CameraDirectoryManager&&) = delete;
    CameraDirectoryManager& operator=(const CameraDirectoryManager&) = delete;
    CameraDirectoryManager& operator=(CameraDirectoryManager&&) = delete;

public:
    // ===== Read =====
    model::CameraGroupArray             getCameraGroups() const;
    model::CameraEntryArray             getCameras() const;
    model::CameraDirectoryRelationArray getCameraRelations() const;
    model::ICameraGroupPtr              getCameraGroup(const std::string& nodeId) const;
    model::ICameraEntryPtr              getCamera(const std::string& nodeId) const;

    // ===== Batch write =====
    model::CameraGroupArray addCameraGroups(const model::CameraGroupArray& groups);
    model::CameraGroupArray updateCameraGroups(const model::CameraGroupArray& groups);
    std::vector<std::string> removeCameraGroups(const std::vector<std::string>& nodeIds);

    model::CameraEntryArray addCameras(const model::CameraEntryArray& cameras);
    model::CameraEntryArray updateCameras(const model::CameraEntryArray& cameras);
    std::vector<std::string> removeCameras(const std::vector<std::string>& nodeIds);

    model::CameraDirectoryRelationArray addCameraRelations(const model::CameraDirectoryRelationArray& relations);
    model::CameraDirectoryRelationArray updateCameraRelations(const model::CameraDirectoryRelationArray& relations);
    std::vector<std::string>            removeCameraRelations(const std::vector<std::string>& childIds);

    // ===== Lifecycle =====
    void bindDatabase(const std::string& databaseId);
    void loadCameraDirectory();
    bool isCameraDirectoryReady() const;

    // Forwarded to the Model; Manager also keeps a weak_ptr copy so it can later emit
    // business-level aggregated events on its own.
    void setNotificationSink(std::weak_ptr<ICameraDirectoryNotificationSink> sink);

private:
    const std::unique_ptr<CameraDirectoryModel> mCameraDirectoryModel;

    // Manager does not emit events today, but the sink reference is reserved for
    // future business-level aggregation use.
    std::weak_ptr<ICameraDirectoryNotificationSink> mNotificationSink;
};

} // namespace ucf::service
