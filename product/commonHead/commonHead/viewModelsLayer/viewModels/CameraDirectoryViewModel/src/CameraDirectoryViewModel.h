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

    // ICameraDirectoryServiceCallback
    void onCameraDirectoryReady() override;
    void onCameraGroupsAdded(const ucf::service::model::CameraGroupArray& groups) override;
    void onCameraGroupsUpdated(const ucf::service::model::CameraGroupArray& groups) override;
    void onCameraGroupsRemoved(const std::vector<std::string>& nodeIds) override;
    void onCamerasAdded(const ucf::service::model::CameraEntryArray& cameras) override;
    void onCamerasUpdated(const ucf::service::model::CameraEntryArray& cameras) override;
    void onCamerasRemoved(const std::vector<std::string>& nodeIds) override;
    void onCameraRelationsAdded(const ucf::service::model::CameraDirectoryRelationArray& relations) override;
    void onCameraRelationsUpdated(const ucf::service::model::CameraDirectoryRelationArray& relations) override;
    void onCameraRelationsRemoved(const std::vector<std::string>& childIds) override;

protected:
    void init() override;

private:
    std::shared_ptr<ucf::service::ICameraDirectoryService> lockService() const;
    void rebuildTreeFromService();

private:
    mutable std::mutex mTreeMutex;
    std::shared_ptr<model::CameraDirectoryTree> mTree;
};

}
