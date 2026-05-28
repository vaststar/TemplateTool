#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "CameraDirectoryEntities.h"

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service {

// 仅负责数据库读写：把内存对象 ↔ 数据库行做转换。不持有任何缓存。
class CameraDirectoryDBAccess
{
public:
    using LoadGroupsCallback    = std::function<void(const model::CameraGroupArray&)>;
    using LoadCamerasCallback   = std::function<void(const model::CameraEntryArray&)>;
    using LoadRelationsCallback = std::function<void(const model::CameraDirectoryRelationArray&)>;

    explicit CameraDirectoryDBAccess(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ~CameraDirectoryDBAccess() = default;
    CameraDirectoryDBAccess(const CameraDirectoryDBAccess&) = delete;
    CameraDirectoryDBAccess(CameraDirectoryDBAccess&&) = delete;
    CameraDirectoryDBAccess& operator=(const CameraDirectoryDBAccess&) = delete;
    CameraDirectoryDBAccess& operator=(CameraDirectoryDBAccess&&) = delete;

public:
    void setDatabaseId(const std::string& dbId);

    // ===== Load (async) =====
    void loadCameraGroups(LoadGroupsCallback callback) const;
    void loadCameras(LoadCamerasCallback callback) const;
    void loadCameraRelations(LoadRelationsCallback callback) const;

    // ===== Persist (sync) =====
    void insertCameraGroups(const model::CameraGroupArray& groups) const;
    void updateCameraGroup(const model::ICameraGroupPtr& group) const;
    void deleteCameraGroup(const std::string& nodeId) const;

    void insertCameras(const model::CameraEntryArray& cameras) const;
    void updateCamera(const model::ICameraEntryPtr& entry) const;
    void deleteCamera(const std::string& nodeId) const;

    void insertCameraRelations(const model::CameraDirectoryRelationArray& relations) const;
    void updateCameraRelation(const model::ICameraDirectoryRelationPtr& relation) const;
    void deleteCameraRelation(const std::string& childId) const;

private:
    std::string resolveDatabaseId() const;

private:
    const ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;
    std::string mDatabaseId;  // 由 Manager 在 DB ready 时设置；未设置则查询 ClientInfoService
};

} // namespace ucf::service
