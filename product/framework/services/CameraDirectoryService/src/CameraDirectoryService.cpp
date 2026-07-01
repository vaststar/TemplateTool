#include "CameraDirectoryService.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Services/DataWarehouseService/IDataWarehouseService.h>
#include <ucf/Services/MediaService/IMediaService.h>
#include <ucf/Services/ClientInfoService/IClientInfoService.h>

#include "CameraDirectoryManager.h"
#include "CameraDirectoryServiceLogger.h"

namespace ucf::service {

/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic///////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

class CameraDirectoryService::DataPrivate
{
public:
    explicit DataPrivate(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ucf::framework::ICoreFrameworkWPtr getCoreFramework() const;
    CameraDirectoryManager& getManager();
    const CameraDirectoryManager& getManager() const;
private:
    ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;
    std::unique_ptr<CameraDirectoryManager> mManager;
};

CameraDirectoryService::DataPrivate::DataPrivate(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFrameworkWPtr(coreFramework)
    , mManager(std::make_unique<CameraDirectoryManager>(coreFramework))
{
}

ucf::framework::ICoreFrameworkWPtr CameraDirectoryService::DataPrivate::getCoreFramework() const
{
    return mCoreFrameworkWPtr;
}

CameraDirectoryManager& CameraDirectoryService::DataPrivate::getManager()
{
    return *mManager;
}

const CameraDirectoryManager& CameraDirectoryService::DataPrivate::getManager() const
{
    return *mManager;
}

/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start CameraDirectoryService Logic////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<ICameraDirectoryService> ICameraDirectoryService::createInstance(ucf::framework::ICoreFrameworkWPtr coreFramework)
{
    return std::make_shared<CameraDirectoryService>(coreFramework);
}

CameraDirectoryService::CameraDirectoryService(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mDataPrivate(std::make_unique<DataPrivate>(coreFramework))
{
    SERVICE_LOG_DEBUG("Create CameraDirectoryService, address:" << this);
}

CameraDirectoryService::~CameraDirectoryService()
{
    SERVICE_LOG_DEBUG("Delete CameraDirectoryService, address:" << this);
}

void CameraDirectoryService::initService()
{
    SERVICE_LOG_DEBUG("CameraDirectoryService::initService()");
    // Inject the sink first: must happen before registering any external callback that
    // could trigger a change, otherwise an early callback would arrive before the sink
    // is in place and events would be lost.
    auto self = shared_from_this();
    mDataPrivate->getManager().setNotificationSink(
        std::static_pointer_cast<ICameraDirectoryNotificationSink>(self));

    if (auto coreFramework = mDataPrivate->getCoreFramework().lock())
    {
        coreFramework->registerCallback(shared_from_this());
        if (auto dataWarehouseService = coreFramework->getService<ucf::service::IDataWarehouseService>().lock())
        {
            dataWarehouseService->registerCallback(shared_from_this());
        }
    }
}

void CameraDirectoryService::deinitService()
{
    SERVICE_LOG_DEBUG("CameraDirectoryService::deinitService()");
    if (auto coreFramework = mDataPrivate->getCoreFramework().lock())
    {
        coreFramework->unRegisterCallback(shared_from_this());
        if (auto dataWarehouseService = coreFramework->getService<ucf::service::IDataWarehouseService>().lock())
        {
            dataWarehouseService->unRegisterCallback(shared_from_this());
        }
    }
}

std::string CameraDirectoryService::getServiceName() const
{
    return "CameraDirectoryService";
}

std::vector<ServiceDependency> CameraDirectoryService::dependencies() const
{
    return {
        { std::type_index(typeid(IMediaService)), DependencyKind::Required },
        { std::type_index(typeid(IDataWarehouseService)), DependencyKind::Required },
        { std::type_index(typeid(IClientInfoService)), DependencyKind::Required }
    };
}

void CameraDirectoryService::onServiceInitialized()
{
    SERVICE_LOG_DEBUG("CameraDirectoryService initialized");
}

void CameraDirectoryService::onCoreFrameworkExit()
{
    SERVICE_LOG_DEBUG("CameraDirectoryService exiting");
}

void CameraDirectoryService::OnDatabaseInitialized(const std::string& dbId)
{
    mDataPrivate->getManager().bindDatabase(dbId);
}

// ===== Read =====
model::CameraGroupArray CameraDirectoryService::getCameraGroups() const
{
    return mDataPrivate->getManager().getCameraGroups();
}

model::CameraEntryArray CameraDirectoryService::getCameras() const
{
    return mDataPrivate->getManager().getCameras();
}

model::CameraDirectoryRelationArray CameraDirectoryService::getCameraRelations() const
{
    return mDataPrivate->getManager().getCameraRelations();
}

model::ICameraGroupPtr CameraDirectoryService::getCameraGroup(const std::string& nodeId) const
{
    return mDataPrivate->getManager().getCameraGroup(nodeId);
}

model::ICameraEntryPtr CameraDirectoryService::getCamera(const std::string& nodeId) const
{
    return mDataPrivate->getManager().getCamera(nodeId);
}

// ===== Batch write =====
// Events are not fired here; the Model emits them via the sink after persistence completes,
// which then routes through this class's onGroupsAdded / onCamerasAdded / ... to translate
// into outward notifications.
void CameraDirectoryService::addCameraGroups(const model::CameraGroupArray& groups)
{
    mDataPrivate->getManager().addCameraGroups(groups);
}

void CameraDirectoryService::updateCameraGroups(const model::CameraGroupArray& groups)
{
    mDataPrivate->getManager().updateCameraGroups(groups);
}

void CameraDirectoryService::removeCameraGroups(const std::vector<std::string>& nodeIds)
{
    mDataPrivate->getManager().removeCameraGroups(nodeIds);
}

void CameraDirectoryService::addCameras(const model::CameraEntryArray& cameras)
{
    mDataPrivate->getManager().addCameras(cameras);
}

void CameraDirectoryService::updateCameras(const model::CameraEntryArray& cameras)
{
    mDataPrivate->getManager().updateCameras(cameras);
}

void CameraDirectoryService::removeCameras(const std::vector<std::string>& nodeIds)
{
    mDataPrivate->getManager().removeCameras(nodeIds);
}

void CameraDirectoryService::addCameraRelations(const model::CameraDirectoryRelationArray& relations)
{
    mDataPrivate->getManager().addCameraRelations(relations);
}

void CameraDirectoryService::updateCameraRelations(const model::CameraDirectoryRelationArray& relations)
{
    mDataPrivate->getManager().updateCameraRelations(relations);
}

void CameraDirectoryService::removeCameraRelations(const std::vector<std::string>& relationIds)
{
    mDataPrivate->getManager().removeCameraRelations(relationIds);
}

// ===== Lifecycle =====
void CameraDirectoryService::loadCameraDirectory()
{
    mDataPrivate->getManager().loadCameraDirectory();
}

bool CameraDirectoryService::isCameraDirectoryReady() const
{
    return mDataPrivate->getManager().isCameraDirectoryReady();
}

// ===== ICameraDirectoryNotificationSink =====
void CameraDirectoryService::onGroupsAdded(const model::CameraGroupArray& groups, CameraDirectoryNotificationSource /*src*/)
{
    fireNotification(&ICameraDirectoryServiceCallback::onCameraGroupsAdded, groups);
}

void CameraDirectoryService::onGroupsUpdated(const model::CameraGroupArray& groups, CameraDirectoryNotificationSource /*src*/)
{
    fireNotification(&ICameraDirectoryServiceCallback::onCameraGroupsUpdated, groups);
}

void CameraDirectoryService::onGroupsRemoved(const std::vector<std::string>& nodeIds, CameraDirectoryNotificationSource /*src*/)
{
    fireNotification(&ICameraDirectoryServiceCallback::onCameraGroupsRemoved, nodeIds);
}

void CameraDirectoryService::onCamerasAdded(const model::CameraEntryArray& cameras, CameraDirectoryNotificationSource /*src*/)
{
    fireNotification(&ICameraDirectoryServiceCallback::onCamerasAdded, cameras);
}

void CameraDirectoryService::onCamerasUpdated(const model::CameraEntryArray& cameras, CameraDirectoryNotificationSource /*src*/)
{
    fireNotification(&ICameraDirectoryServiceCallback::onCamerasUpdated, cameras);
}

void CameraDirectoryService::onCamerasRemoved(const std::vector<std::string>& nodeIds, CameraDirectoryNotificationSource /*src*/)
{
    fireNotification(&ICameraDirectoryServiceCallback::onCamerasRemoved, nodeIds);
}

void CameraDirectoryService::onRelationsAdded(const model::CameraDirectoryRelationArray& relations, CameraDirectoryNotificationSource /*src*/)
{
    fireNotification(&ICameraDirectoryServiceCallback::onCameraRelationsAdded, relations);
}

void CameraDirectoryService::onRelationsUpdated(const model::CameraDirectoryRelationArray& relations, CameraDirectoryNotificationSource /*src*/)
{
    fireNotification(&ICameraDirectoryServiceCallback::onCameraRelationsUpdated, relations);
}

void CameraDirectoryService::onRelationsRemoved(const std::vector<std::string>& relationIds, CameraDirectoryNotificationSource /*src*/)
{
    fireNotification(&ICameraDirectoryServiceCallback::onCameraRelationsRemoved, relationIds);
}

void CameraDirectoryService::onDirectoryLoaded()
{
    fireNotification(&ICameraDirectoryServiceCallback::onCameraDirectoryReady);
}

void CameraDirectoryService::onDirectoryLoadFailed(CameraDirectoryLoadError error)
{
    fireNotification(&ICameraDirectoryServiceCallback::onCameraDirectoryLoadFailed, error);
}

/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish CameraDirectoryService Logic///////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

} // namespace ucf::service
