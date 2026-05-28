#pragma once

#include <memory>
#include <string>
#include <vector>

#include "CameraDirectoryEntities.h"

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service {

class CameraDirectoryModel;

// 业务编排层：当前仅转发到 CameraDirectoryModel；
// 预留位置用于后续承担摄像头发现 / 设备热插拔 / 默认目录初始化等业务。
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
    void onDatabaseReady(const std::string& databaseId);

private:
    const std::unique_ptr<CameraDirectoryModel> mCameraDirectoryModel;
};

} // namespace ucf::service
