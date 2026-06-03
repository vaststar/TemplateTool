#include "CameraDirectoryManager.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Services/ClientInfoService/IClientInfoService.h>
#include <ucf/Services/DataWarehouseService/DatabaseConfig.h>

#include "CameraDirectoryModel.h"
#include "CameraDirectoryServiceLogger.h"

namespace ucf::service {

CameraDirectoryManager::CameraDirectoryManager(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFrameworkWPtr(coreFramework)
    , mCameraDirectoryModel(std::make_unique<CameraDirectoryModel>(coreFramework))
{
    SERVICE_LOG_DEBUG("Create CameraDirectoryManager, address:" << this);
}

CameraDirectoryManager::~CameraDirectoryManager()
{
    SERVICE_LOG_DEBUG("Delete CameraDirectoryManager, address:" << this);
}

// ===== Read =====

model::CameraGroupArray CameraDirectoryManager::getCameraGroups() const
{
    return mCameraDirectoryModel->getCameraGroups();
}

model::CameraEntryArray CameraDirectoryManager::getCameras() const
{
    return mCameraDirectoryModel->getCameras();
}

model::CameraDirectoryRelationArray CameraDirectoryManager::getCameraRelations() const
{
    return mCameraDirectoryModel->getCameraRelations();
}

model::ICameraGroupPtr CameraDirectoryManager::getCameraGroup(const std::string& nodeId) const
{
    return mCameraDirectoryModel->getCameraGroup(nodeId);
}

model::ICameraEntryPtr CameraDirectoryManager::getCamera(const std::string& nodeId) const
{
    return mCameraDirectoryModel->getCamera(nodeId);
}

// ===== Group writes =====

model::CameraGroupArray CameraDirectoryManager::addCameraGroups(const model::CameraGroupArray& groups)
{
    return mCameraDirectoryModel->addCameraGroups(groups);
}

model::CameraGroupArray CameraDirectoryManager::updateCameraGroups(const model::CameraGroupArray& groups)
{
    return mCameraDirectoryModel->updateCameraGroups(groups);
}

std::vector<std::string> CameraDirectoryManager::removeCameraGroups(const std::vector<std::string>& nodeIds)
{
    return mCameraDirectoryModel->removeCameraGroups(nodeIds);
}

// ===== Camera writes =====

model::CameraEntryArray CameraDirectoryManager::addCameras(const model::CameraEntryArray& cameras)
{
    return mCameraDirectoryModel->addCameras(cameras);
}

model::CameraEntryArray CameraDirectoryManager::updateCameras(const model::CameraEntryArray& cameras)
{
    return mCameraDirectoryModel->updateCameras(cameras);
}

std::vector<std::string> CameraDirectoryManager::removeCameras(const std::vector<std::string>& nodeIds)
{
    return mCameraDirectoryModel->removeCameras(nodeIds);
}

// ===== Relation writes =====

model::CameraDirectoryRelationArray CameraDirectoryManager::addCameraRelations(const model::CameraDirectoryRelationArray& relations)
{
    return mCameraDirectoryModel->addCameraRelations(relations);
}

model::CameraDirectoryRelationArray CameraDirectoryManager::updateCameraRelations(const model::CameraDirectoryRelationArray& relations)
{
    return mCameraDirectoryModel->updateCameraRelations(relations);
}

std::vector<std::string> CameraDirectoryManager::removeCameraRelations(const std::vector<std::string>& childIds)
{
    return mCameraDirectoryModel->removeCameraRelations(childIds);
}

// ===== Lifecycle =====

void CameraDirectoryManager::bindDatabase(const std::string& databaseId)
{
    // Filter: DataWarehouseService may host other DBs; only act on the shared DB
    // owned by ClientInfoService (single source of truth for the shared db id).
    auto coreFramework = mCoreFrameworkWPtr.lock();
    if (!coreFramework)
    {
        SERVICE_LOG_ERROR("bindDatabase ignored: CoreFramework unavailable");
        return;
    }
    auto clientInfoService = coreFramework->getService<ucf::service::IClientInfoService>().lock();
    if (!clientInfoService)
    {
        SERVICE_LOG_ERROR("bindDatabase ignored: ClientInfoService unavailable");
        return;
    }
    if (const auto sharedDbId = clientInfoService->getSharedDBConfig().getDBId(); databaseId != sharedDbId)
    {
        SERVICE_LOG_DEBUG("bindDatabase ignored, not shared db, incoming:" << databaseId
                          << ", shared:" << sharedDbId);
        return;
    }

    SERVICE_LOG_DEBUG("Binding CameraDirectoryModel to databaseId:" << databaseId);
    mCameraDirectoryModel->bindDatabase(databaseId);
}

void CameraDirectoryManager::loadCameraDirectory()
{
    mCameraDirectoryModel->loadCameraDirectory();
}

bool CameraDirectoryManager::isCameraDirectoryReady() const
{
    return mCameraDirectoryModel->isCameraDirectoryReady();
}

void CameraDirectoryManager::setNotificationSink(std::weak_ptr<ICameraDirectoryNotificationSink> sink)
{
    mNotificationSink = sink;
    mCameraDirectoryModel->setNotificationSink(std::move(sink));
}

} // namespace ucf::service
