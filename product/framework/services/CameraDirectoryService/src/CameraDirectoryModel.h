#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "CameraDirectoryEntities.h"

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service {

class CameraDirectoryDBAccess;

// 内存数据视图 + 数据库持久化的协调点：
//   - 内存层：批量 CRUD，加锁保护
//   - 持久层：内部持有 CameraDirectoryDBAccess，对接受到的写入做落盘
class CameraDirectoryModel
{
public:
    explicit CameraDirectoryModel(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ~CameraDirectoryModel();
    CameraDirectoryModel(const CameraDirectoryModel&) = delete;
    CameraDirectoryModel(CameraDirectoryModel&&) = delete;
    CameraDirectoryModel& operator=(const CameraDirectoryModel&) = delete;
    CameraDirectoryModel& operator=(CameraDirectoryModel&&) = delete;

public:
    // ===== Read =====
    model::CameraGroupArray             getCameraGroups() const;
    model::CameraEntryArray             getCameras() const;
    model::CameraDirectoryRelationArray getCameraRelations() const;
    model::ICameraGroupPtr              getCameraGroup(const std::string& nodeId) const;
    model::ICameraEntryPtr              getCamera(const std::string& nodeId) const;

    // ===== Batch write: 先 memory 后 DB，返回真正生效的项 =====
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
    // memory-only primitives（不触发 DB）
    model::CameraGroupArray             addCameraGroupsInMemory(const model::CameraGroupArray& groups);
    model::CameraGroupArray             updateCameraGroupsInMemory(const model::CameraGroupArray& groups);
    std::vector<std::string>            removeCameraGroupsInMemory(const std::vector<std::string>& nodeIds);
    model::CameraEntryArray             addCamerasInMemory(const model::CameraEntryArray& cameras);
    model::CameraEntryArray             updateCamerasInMemory(const model::CameraEntryArray& cameras);
    std::vector<std::string>            removeCamerasInMemory(const std::vector<std::string>& nodeIds);
    model::CameraDirectoryRelationArray addCameraRelationsInMemory(const model::CameraDirectoryRelationArray& relations);
    model::CameraDirectoryRelationArray updateCameraRelationsInMemory(const model::CameraDirectoryRelationArray& relations);
    std::vector<std::string>            removeCameraRelationsInMemory(const std::vector<std::string>& childIds);

private:
    mutable std::mutex mMutex;
    std::unordered_map<std::string, std::shared_ptr<model::CameraGroupImpl>>             mGroups;
    std::unordered_map<std::string, std::shared_ptr<model::CameraEntryImpl>>             mCameras;
    std::unordered_map<std::string, std::shared_ptr<model::CameraDirectoryRelationImpl>> mRelations;

    const std::unique_ptr<CameraDirectoryDBAccess> mCameraDirectoryDBAccess;
};

} // namespace ucf::service
