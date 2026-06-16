#pragma once

#include <memory>
#include <mutex>
#include <string>

#include <ucf/Services/CameraDirectoryService/ICameraDirectoryServiceCallback.h>

namespace ucf::service {
    class ICameraDirectoryService;
}

#include <commonHead/commonHeadUtils/VMNotificationHelper/VMNotificationHelper.h>
#include <commonHead/viewModels/CameraDirectoryViewModel/ICameraDirectoryViewModel.h>

#include "CameraDirectoryTreeModel.h"

namespace commonHead::viewModels {

class CameraDirectoryViewModel
    : public virtual ICameraDirectoryViewModel
    , public virtual commonHead::utilities::VMNotificationHelper<ICameraDirectoryViewModelCallback>
    , public ucf::service::ICameraDirectoryServiceCallback
    , public std::enable_shared_from_this<CameraDirectoryViewModel>
{
public:
    explicit CameraDirectoryViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
    CameraDirectoryViewModel() = default;
    CameraDirectoryViewModel(const CameraDirectoryViewModel&) = delete;
    CameraDirectoryViewModel(CameraDirectoryViewModel&&) = delete;
    CameraDirectoryViewModel& operator=(const CameraDirectoryViewModel&) = delete;
    CameraDirectoryViewModel& operator=(CameraDirectoryViewModel&&) = delete;
    ~CameraDirectoryViewModel() = default;

public:
    // IViewModel
    std::string getViewModelName() const override;

    // ICameraDirectoryViewModel - read
    model::CameraDirectoryTreePtr getCameraTree() const override;
    std::optional<model::CameraSource> getCameraSource(const std::string& nodeId) const override;
    bool isCameraDirectoryReady() const override;
    std::string getCurrentCameraId() const override;

    // ICameraDirectoryViewModel - create / remove permissions
    bool canAddCameraNode(const std::string& parentId,
                          model::CameraDirectoryNodeType type) const override;
    bool canRemoveCameraNode(const std::string& nodeId) const override;

    // ICameraDirectoryViewModel - write groups
    void addCameraGroup(const std::string& nodeId, const std::string& displayName) override;
    void updateCameraGroup(const std::string& nodeId, const std::string& displayName) override;
    void removeCameraGroups(const std::vector<std::string>& nodeIds) override;
    // ICameraDirectoryViewModel - write cameras
    void addCamera(const std::string& nodeId,
                   const std::string& displayName,
                   const model::CameraSource& source) override;
    void updateCamera(const std::string& nodeId,
                      const std::string& displayName,
                      const model::CameraSource& source) override;
    void removeCameras(const std::vector<std::string>& nodeIds) override;

    // ICameraDirectoryViewModel - write relations
    void addRelation(const std::string& parentId, const std::string& childId) override;
    void updateRelation(const std::string& parentId, const std::string& childId) override;
    void removeRelations(const std::vector<std::string>& childIds) override;

    // ICameraDirectoryViewModel - drag-drop / re-parent
    bool canMoveCameraNode(const std::string& childId, const std::string& newParentId) const override;
    void moveCameraNode(const std::string& childId, const std::string& newParentId) override;

    // ICameraDirectoryViewModel - selection
    void selectCamera(const std::string& nodeId) override;

    // ICameraDirectoryServiceCallback
    void onCameraDirectoryReady() override;
    void onCameraDirectoryLoadFailed(ucf::service::CameraDirectoryLoadError error) override;
    void onCameraGroupsAdded(const ucf::service::model::CameraGroupArray& groups) override;
    void onCameraGroupsUpdated(const ucf::service::model::CameraGroupArray& groups) override;
    void onCameraGroupsRemoved(const std::vector<std::string>& nodeIds) override;
    void onCamerasAdded(const ucf::service::model::CameraEntryArray& cameras) override;
    void onCamerasUpdated(const ucf::service::model::CameraEntryArray& cameras) override;
    void onCamerasRemoved(const std::vector<std::string>& nodeIds) override;
    void onCameraRelationsAdded(const ucf::service::model::CameraDirectoryRelationArray& relations) override;
    void onCameraRelationsUpdated(const ucf::service::model::CameraDirectoryRelationArray& relations) override;
    void onCameraRelationsRemoved(const std::vector<std::string>& relationIds) override;

protected:
    void init() override;

private:
    std::shared_ptr<ucf::service::ICameraDirectoryService> lockService() const;
    void rebuildTreeFromService();
    // Returns true when the tree was just rebuilt (because it was null), in which case
    // the incremental apply should be skipped because the rebuilt tree already contains
    // the event's effect.
    bool ensureTreeBuilt();
    // Snapshot the current tree under lock (may be null before the first load); callers
    // then operate on the snapshot without holding mTreeMutex.
    std::shared_ptr<model::CameraDirectoryTree> snapshotTree() const;
    // Find a node by id within the given snapshot; a null tree or unknown id yields nullptr.
    model::CameraDirectoryTreeNodePtr findNode(const std::shared_ptr<model::CameraDirectoryTree>& tree,
                                               const std::string& id) const;

private:
    mutable std::mutex mTreeMutex;
    std::shared_ptr<model::CameraDirectoryTree> mTree;

    mutable std::mutex mSelectionMutex;
    std::string mCurrentCameraId;
};

}
