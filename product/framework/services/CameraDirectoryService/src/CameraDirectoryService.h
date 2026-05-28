#pragma once

#include <memory>

#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>
#include <ucf/CoreFramework/CoreFrameworkCallbackDefault.h>
#include <ucf/Services/CameraDirectoryService/ICameraDirectoryService.h>
#include <ucf/Services/DataWarehouseService/IDataWarehouseServiceCallback.h>

namespace ucf::service {

class SERVICE_EXPORT CameraDirectoryService final
    : public virtual ICameraDirectoryService
    , public virtual ucf::utilities::NotificationHelper<ICameraDirectoryServiceCallback>
    , public ucf::framework::CoreFrameworkCallbackDefault
    , public ucf::service::IDataWarehouseServiceCallback
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

protected:
    // IService
    virtual void initService() override;

private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};

} // namespace ucf::service
