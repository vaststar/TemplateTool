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
    // Directory data is fully loaded (the service finished its initial DB-driven load and
    // this VM has populated its tree). Only fired once per successful load; subscribers
    // that join after the load is already done should call isCameraDirectoryReady() to
    // detect that state instead of waiting for this callback.
    virtual void onCameraDirectoryReady() {}

    // Directory load failed; the VM keeps whatever data it had and the caller can retry
    // by triggering a reload through the service.
    virtual void onCameraDirectoryLoadFailed(model::CameraDirectoryLoadError /*error*/) {}

    // ===== Incremental node events (same shape as the service callbacks, payload translated
    // to the VM types). =====
    virtual void onCameraGroupsAdded(const std::vector<model::CameraDirectoryNodeData>& /*groups*/) {}
    virtual void onCameraGroupsUpdated(const std::vector<model::CameraDirectoryNodeData>& /*groups*/) {}
    virtual void onCameraGroupsRemoved(const std::vector<std::string>& /*nodeIds*/) {}

    virtual void onCamerasAdded(const std::vector<model::CameraDirectoryNodeData>& /*cameras*/) {}
    virtual void onCamerasUpdated(const std::vector<model::CameraDirectoryNodeData>& /*cameras*/) {}
    virtual void onCamerasRemoved(const std::vector<std::string>& /*nodeIds*/) {}

    // ===== Incremental relation events. =====
    virtual void onCameraRelationsAdded(const std::vector<model::CameraDirectoryRelationData>& /*relations*/) {}
    virtual void onCameraRelationsUpdated(const std::vector<model::CameraDirectoryRelationData>& /*relations*/) {}
    virtual void onCameraRelationsRemoved(const std::vector<std::string>& /*childIds*/) {}

    // The VM-owned current selection changed. Empty nodeId means selection was cleared
    // (e.g. the previously selected camera was removed). Fired after de-duplication, so
    // subscribers can treat every invocation as a real state change.
    virtual void onCurrentCameraChanged(const std::string& /*nodeId*/) {}
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

    // True after the VM tree has been populated from a successful service load. Late
    // subscribers should check this on init / on registration; if true, they should read
    // the current tree directly because onCameraDirectoryReady will not be re-fired.
    [[nodiscard]] virtual bool isCameraDirectoryReady() const = 0;

    // The current selected camera node id (empty if none). This is VM-owned UI state
    // shared by all subscribers of this VM instance. Multi-window setups create separate
    // VM instances and therefore have independent selections; the underlying Service is
    // shared but does not store selection.
    [[nodiscard]] virtual std::string getCurrentCameraId() const = 0;

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

    // ===== Selection =====
    // Set the current selected camera node. Pass an empty string to clear selection.
    // Non-camera ids and unknown ids are ignored. Same-as-current calls are deduped.
    // Successful changes fire onCurrentCameraChanged; this is the single funnel point
    // for selection metrics / analytics.
    virtual void selectCamera(const std::string& nodeId) = 0;

public:
    static std::shared_ptr<ICameraDirectoryViewModel> createInstance(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
};

}
