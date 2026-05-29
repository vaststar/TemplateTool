#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>
#include <commonHead/commonHeadUtils/VMNotificationHelper/IVMNotificationHelper.h>
#include <commonHead/viewModels/IViewModel/IViewModel.h>
#include <commonHead/viewModels/MediaCameraViewModel/CameraSource.h>

#include <commonHead/viewModels/CameraDirectoryViewModel/ICameraDirectoryTreeModel.h>

namespace commonHead {
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWptr = std::weak_ptr<ICommonHeadFramework>;
}

namespace commonHead::viewModels {

class COMMONHEAD_EXPORT ICameraDirectoryViewModelCallback
{
public:
    ICameraDirectoryViewModelCallback() = default;
    ICameraDirectoryViewModelCallback(const ICameraDirectoryViewModelCallback&) = delete;
    ICameraDirectoryViewModelCallback(ICameraDirectoryViewModelCallback&&) = delete;
    ICameraDirectoryViewModelCallback& operator=(const ICameraDirectoryViewModelCallback&) = delete;
    ICameraDirectoryViewModelCallback& operator=(ICameraDirectoryViewModelCallback&&) = delete;
    virtual ~ICameraDirectoryViewModelCallback() = default;

public:
    // 目录可用（service 数据库就绪、首次构树完成）
    virtual void onCameraDirectoryReady() {}

    // ===== 增量节点事件（与 service 回调形状一致，仅类型转为 VM 层）=====
    virtual void onCameraGroupsAdded(const std::vector<model::CameraDirectoryNodeData>& /*groups*/) {}
    virtual void onCameraGroupsUpdated(const std::vector<model::CameraDirectoryNodeData>& /*groups*/) {}
    virtual void onCameraGroupsRemoved(const std::vector<std::string>& /*nodeIds*/) {}

    virtual void onCamerasAdded(const std::vector<model::CameraDirectoryNodeData>& /*cameras*/) {}
    virtual void onCamerasUpdated(const std::vector<model::CameraDirectoryNodeData>& /*cameras*/) {}
    virtual void onCamerasRemoved(const std::vector<std::string>& /*nodeIds*/) {}

    // ===== 增量关系事件 =====
    virtual void onCameraRelationsAdded(const std::vector<model::CameraDirectoryRelationData>& /*relations*/) {}
    virtual void onCameraRelationsUpdated(const std::vector<model::CameraDirectoryRelationData>& /*relations*/) {}
    virtual void onCameraRelationsRemoved(const std::vector<std::string>& /*childIds*/) {}
};

class COMMONHEAD_EXPORT ICameraDirectoryViewModel
    : public IViewModel
    , public virtual commonHead::utilities::IVMNotificationHelper<ICameraDirectoryViewModelCallback>
{
public:
    using IViewModel::IViewModel;
    ICameraDirectoryViewModel(const ICameraDirectoryViewModel&) = delete;
    ICameraDirectoryViewModel(ICameraDirectoryViewModel&&) = delete;
    ICameraDirectoryViewModel& operator=(const ICameraDirectoryViewModel&) = delete;
    ICameraDirectoryViewModel& operator=(ICameraDirectoryViewModel&&) = delete;
    virtual ~ICameraDirectoryViewModel() = default;

public:
    virtual std::string getViewModelName() const = 0;

    // ===== Read =====
    [[nodiscard]] virtual model::CameraDirectoryTreePtr getCameraTree() const = 0;
    [[nodiscard]] virtual std::optional<model::CameraSource> getCameraSource(const std::string& nodeId) const = 0;

    // ===== Write: Groups =====
    virtual void addCameraGroup(const std::string& nodeId, const std::string& displayName) = 0;
    virtual void updateCameraGroup(const std::string& nodeId, const std::string& displayName) = 0;
    virtual void removeCameraGroups(const std::vector<std::string>& nodeIds) = 0;

    // ===== Write: Cameras =====
    virtual void addCamera(const std::string& nodeId,
                           const std::string& displayName,
                           const model::CameraSource& source) = 0;
    virtual void updateCamera(const std::string& nodeId,
                              const std::string& displayName,
                              const model::CameraSource& source) = 0;
    virtual void removeCameras(const std::vector<std::string>& nodeIds) = 0;

    // ===== Write: Relations (each child has exactly one parent) =====
    virtual void addRelation(const std::string& parentId, const std::string& childId) = 0;
    virtual void updateRelation(const std::string& parentId, const std::string& childId) = 0;
    virtual void removeRelations(const std::vector<std::string>& childIds) = 0;

public:
    static std::shared_ptr<ICameraDirectoryViewModel> createInstance(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
};

}
