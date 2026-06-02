#pragma once

#include <memory>

#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>
#include <ucf/CoreFramework/CoreFrameworkCallbackDefault.h>
#include <ucf/Services/CameraDirectoryService/ICameraDirectoryService.h>
#include <ucf/Services/DataWarehouseService/IDataWarehouseServiceCallback.h>

#include "CameraDirectoryNotificationSink.h"

namespace ucf::service {

class SERVICE_EXPORT CameraDirectoryService final
    : public virtual ICameraDirectoryService
    , public virtual ucf::utilities::NotificationHelper<ICameraDirectoryServiceCallback>
    , public ucf::framework::CoreFrameworkCallbackDefault
    , public ucf::service::IDataWarehouseServiceCallback
    , public ICameraDirectoryNotificationSink
    , public std::enable_shared_from_this<CameraDirectoryService>
{
public:
    explicit CameraDirectoryService(ucf::framework::ICoreFrameworkWPtr coreFramework);
    virtual ~CameraDirectoryService();
    CameraDirectoryService(const CameraDirectoryService&) = delete;
    CameraDirectoryService(CameraDirectoryService&&) = delete;
    CameraDirectoryService& operator=(const CameraDirectoryService&) = delete;
    CameraDirectoryService& operator=(CameraDirectoryService&&) = delete;

public:
    // IService
    virtual std::string getServiceName() const override;

    // ICoreFrameworkCallback
    virtual void onServiceInitialized() override;
    virtual void onCoreFrameworkExit() override;

    // IDataWarehouseServiceCallback
    virtual void OnDatabaseInitialized(const std::string& dbId) override;

    // ICameraDirectoryService - Read
    virtual model::CameraGroupArray  getCameraGroups()  const override;
    virtual model::CameraEntryArray  getCameras() const override;
    virtual model::CameraDirectoryRelationArray getCameraRelations() const override;
    virtual model::ICameraGroupPtr   getCameraGroup(const std::string& nodeId) const override;
    virtual model::ICameraEntryPtr   getCamera(const std::string& nodeId) const override;

    // ICameraDirectoryService - Batch write
    virtual void addCameraGroups(const model::CameraGroupArray& groups) override;
    virtual void updateCameraGroups(const model::CameraGroupArray& groups) override;
    virtual void removeCameraGroups(const std::vector<std::string>& nodeIds) override;

    virtual void addCameras(const model::CameraEntryArray& cameras) override;
    virtual void updateCameras(const model::CameraEntryArray& cameras) override;
    virtual void removeCameras(const std::vector<std::string>& nodeIds) override;

    virtual void addCameraRelations(const model::CameraDirectoryRelationArray& relations) override;
    virtual void updateCameraRelations(const model::CameraDirectoryRelationArray& relations) override;
    virtual void removeCameraRelations(const std::vector<std::string>& childIds) override;

    // ICameraDirectoryService - Lifecycle
    virtual void loadCameraDirectory() override;
    virtual bool isCameraDirectoryReady() const override;

protected:
    // IService
    virtual void initService() override;

private:
    // ICameraDirectoryNotificationSink — translation layer from internal changes to outward
    // notifications; not exposed externally.
    virtual void onGroupsAdded   (const model::CameraGroupArray& groups,    CameraDirectoryNotificationSource src) override;
    virtual void onGroupsUpdated (const model::CameraGroupArray& groups,    CameraDirectoryNotificationSource src) override;
    virtual void onGroupsRemoved (const std::vector<std::string>& nodeIds,  CameraDirectoryNotificationSource src) override;
    virtual void onCamerasAdded  (const model::CameraEntryArray& cameras,   CameraDirectoryNotificationSource src) override;
    virtual void onCamerasUpdated(const model::CameraEntryArray& cameras,   CameraDirectoryNotificationSource src) override;
    virtual void onCamerasRemoved(const std::vector<std::string>& nodeIds,  CameraDirectoryNotificationSource src) override;
    virtual void onRelationsAdded  (const model::CameraDirectoryRelationArray& relations, CameraDirectoryNotificationSource src) override;
    virtual void onRelationsUpdated(const model::CameraDirectoryRelationArray& relations, CameraDirectoryNotificationSource src) override;
    virtual void onRelationsRemoved(const std::vector<std::string>& childIds,             CameraDirectoryNotificationSource src) override;
    virtual void onDirectoryLoaded() override;
    virtual void onDirectoryLoadFailed(CameraDirectoryLoadError error) override;

private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};

} // namespace ucf::service
