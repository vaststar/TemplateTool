#include "CameraDirectoryService.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Services/DataWarehouseService/IDataWarehouseService.h>

#include "CameraDirectoryManager.h"
#include "CameraDirectoryServiceLogger.h"

namespace ucf::service {

/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic///////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

class CameraDirectoryService::DataPrivate {
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
    if (auto coreFramework = mDataPrivate->getCoreFramework().lock())
    {
        coreFramework->registerCallback(shared_from_this());
        if (auto dataWarehouseService = coreFramework->getService<ucf::service::IDataWarehouseService>().lock())
        {
            dataWarehouseService->registerCallback(shared_from_this());
        }
    }
}

std::string CameraDirectoryService::getServiceName() const
{
    return "CameraDirectoryService";
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
    mDataPrivate->getManager().onDatabaseReady(dbId);
    fireNotification(&ICameraDirectoryServiceCallback::onCameraDirectoryReady);
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
void CameraDirectoryService::addCameraGroups(const model::CameraGroupArray& groups)
{
    auto accepted = mDataPrivate->getManager().addCameraGroups(groups);
    if (!accepted.empty())
    {
        fireNotification(&ICameraDirectoryServiceCallback::onCameraGroupsAdded, accepted);
    }
}

void CameraDirectoryService::updateCameraGroups(const model::CameraGroupArray& groups)
{
    auto accepted = mDataPrivate->getManager().updateCameraGroups(groups);
    if (!accepted.empty())
    {
        fireNotification(&ICameraDirectoryServiceCallback::onCameraGroupsUpdated, accepted);
    }
}

void CameraDirectoryService::removeCameraGroups(const std::vector<std::string>& nodeIds)
{
    auto accepted = mDataPrivate->getManager().removeCameraGroups(nodeIds);
    if (!accepted.empty())
    {
        fireNotification(&ICameraDirectoryServiceCallback::onCameraGroupsRemoved, accepted);
    }
}

void CameraDirectoryService::addCameras(const model::CameraEntryArray& cameras)
{
    auto accepted = mDataPrivate->getManager().addCameras(cameras);
    if (!accepted.empty())
    {
        fireNotification(&ICameraDirectoryServiceCallback::onCamerasAdded, accepted);
    }
}

void CameraDirectoryService::updateCameras(const model::CameraEntryArray& cameras)
{
    auto accepted = mDataPrivate->getManager().updateCameras(cameras);
    if (!accepted.empty())
    {
        fireNotification(&ICameraDirectoryServiceCallback::onCamerasUpdated, accepted);
    }
}

void CameraDirectoryService::removeCameras(const std::vector<std::string>& nodeIds)
{
    auto accepted = mDataPrivate->getManager().removeCameras(nodeIds);
    if (!accepted.empty())
    {
        fireNotification(&ICameraDirectoryServiceCallback::onCamerasRemoved, accepted);
    }
}

void CameraDirectoryService::addCameraRelations(const model::CameraDirectoryRelationArray& relations)
{
    auto accepted = mDataPrivate->getManager().addCameraRelations(relations);
    if (!accepted.empty())
    {
        fireNotification(&ICameraDirectoryServiceCallback::onCameraRelationsAdded, accepted);
    }
}

void CameraDirectoryService::updateCameraRelations(const model::CameraDirectoryRelationArray& relations)
{
    auto accepted = mDataPrivate->getManager().updateCameraRelations(relations);
    if (!accepted.empty())
    {
        fireNotification(&ICameraDirectoryServiceCallback::onCameraRelationsUpdated, accepted);
    }
}

void CameraDirectoryService::removeCameraRelations(const std::vector<std::string>& childIds)
{
    auto accepted = mDataPrivate->getManager().removeCameraRelations(childIds);
    if (!accepted.empty())
    {
        fireNotification(&ICameraDirectoryServiceCallback::onCameraRelationsRemoved, accepted);
    }
}

/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish CameraDirectoryService Logic///////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

} // namespace ucf::service
