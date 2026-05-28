#include "CameraDirectoryModel.h"

#include "CameraDirectoryDBAccess.h"
#include "CameraDirectoryServiceLogger.h"

namespace ucf::service {

CameraDirectoryModel::CameraDirectoryModel(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCameraDirectoryDBAccess(std::make_unique<CameraDirectoryDBAccess>(coreFramework))
{
}

CameraDirectoryModel::~CameraDirectoryModel() = default;

// ===== Read =====

model::CameraGroupArray CameraDirectoryModel::getCameraGroups() const
{
    model::CameraGroupArray result;
    std::scoped_lock lock(mMutex);
    result.reserve(mGroups.size());
    for (const auto& [_, group] : mGroups)
    {
        result.push_back(group);
    }
    return result;
}

model::CameraEntryArray CameraDirectoryModel::getCameras() const
{
    model::CameraEntryArray result;
    std::scoped_lock lock(mMutex);
    result.reserve(mCameras.size());
    for (const auto& [_, entry] : mCameras)
    {
        result.push_back(entry);
    }
    return result;
}

model::CameraDirectoryRelationArray CameraDirectoryModel::getCameraRelations() const
{
    model::CameraDirectoryRelationArray result;
    std::scoped_lock lock(mMutex);
    result.reserve(mRelations.size());
    for (const auto& [_, relation] : mRelations)
    {
        result.push_back(relation);
    }
    return result;
}

model::ICameraGroupPtr CameraDirectoryModel::getCameraGroup(const std::string& nodeId) const
{
    std::scoped_lock lock(mMutex);
    auto it = mGroups.find(nodeId);
    if (it == mGroups.end())
    {
        return nullptr;
    }
    return it->second;
}

model::ICameraEntryPtr CameraDirectoryModel::getCamera(const std::string& nodeId) const
{
    std::scoped_lock lock(mMutex);
    auto it = mCameras.find(nodeId);
    if (it == mCameras.end())
    {
        return nullptr;
    }
    return it->second;
}

// ===== Memory primitives =====

model::CameraGroupArray CameraDirectoryModel::addCameraGroupsInMemory(const model::CameraGroupArray& groups)
{
    model::CameraGroupArray accepted;
    accepted.reserve(groups.size());
    std::scoped_lock lock(mMutex);
    for (const auto& group : groups)
    {
        if (!group)
        {
            continue;
        }
        const std::string nodeId = group->getNodeId();
        if (nodeId.empty() || mGroups.find(nodeId) != mGroups.end())
        {
            continue;
        }
        auto impl = std::make_shared<model::CameraGroupImpl>(
            nodeId, group->getDisplayName(), group->getNodeStatus());
        mGroups.emplace(nodeId, impl);
        accepted.push_back(impl);
    }
    return accepted;
}

model::CameraGroupArray CameraDirectoryModel::updateCameraGroupsInMemory(const model::CameraGroupArray& groups)
{
    model::CameraGroupArray accepted;
    accepted.reserve(groups.size());
    std::scoped_lock lock(mMutex);
    for (const auto& group : groups)
    {
        if (!group)
        {
            continue;
        }
        auto it = mGroups.find(group->getNodeId());
        if (it == mGroups.end())
        {
            continue;
        }
        it->second->setDisplayName(group->getDisplayName());
        it->second->setNodeStatus(group->getNodeStatus());
        accepted.push_back(it->second);
    }
    return accepted;
}

std::vector<std::string> CameraDirectoryModel::removeCameraGroupsInMemory(const std::vector<std::string>& nodeIds)
{
    std::vector<std::string> accepted;
    accepted.reserve(nodeIds.size());
    std::scoped_lock lock(mMutex);
    for (const auto& nodeId : nodeIds)
    {
        if (mGroups.erase(nodeId) > 0)
        {
            accepted.push_back(nodeId);
        }
    }
    return accepted;
}

model::CameraEntryArray CameraDirectoryModel::addCamerasInMemory(const model::CameraEntryArray& cameras)
{
    model::CameraEntryArray accepted;
    accepted.reserve(cameras.size());
    std::scoped_lock lock(mMutex);
    for (const auto& entry : cameras)
    {
        if (!entry)
        {
            continue;
        }
        const std::string nodeId = entry->getNodeId();
        if (nodeId.empty() || mCameras.find(nodeId) != mCameras.end())
        {
            continue;
        }
        auto impl = std::make_shared<model::CameraEntryImpl>(
            nodeId, entry->getDisplayName(), entry->getNodeStatus(), entry->getSource());
        mCameras.emplace(nodeId, impl);
        accepted.push_back(impl);
    }
    return accepted;
}

model::CameraEntryArray CameraDirectoryModel::updateCamerasInMemory(const model::CameraEntryArray& cameras)
{
    model::CameraEntryArray accepted;
    accepted.reserve(cameras.size());
    std::scoped_lock lock(mMutex);
    for (const auto& entry : cameras)
    {
        if (!entry)
        {
            continue;
        }
        auto it = mCameras.find(entry->getNodeId());
        if (it == mCameras.end())
        {
            continue;
        }
        it->second->setDisplayName(entry->getDisplayName());
        it->second->setNodeStatus(entry->getNodeStatus());
        it->second->setSource(entry->getSource());
        accepted.push_back(it->second);
    }
    return accepted;
}

std::vector<std::string> CameraDirectoryModel::removeCamerasInMemory(const std::vector<std::string>& nodeIds)
{
    std::vector<std::string> accepted;
    accepted.reserve(nodeIds.size());
    std::scoped_lock lock(mMutex);
    for (const auto& nodeId : nodeIds)
    {
        if (mCameras.erase(nodeId) > 0)
        {
            accepted.push_back(nodeId);
        }
    }
    return accepted;
}

model::CameraDirectoryRelationArray CameraDirectoryModel::addCameraRelationsInMemory(const model::CameraDirectoryRelationArray& relations)
{
    model::CameraDirectoryRelationArray accepted;
    accepted.reserve(relations.size());
    std::scoped_lock lock(mMutex);
    for (const auto& relation : relations)
    {
        if (!relation)
        {
            continue;
        }
        const std::string parentId = relation->getParentId();
        const std::string childId  = relation->getChildId();
        if (parentId.empty() || childId.empty())
        {
            continue;
        }
        if (mRelations.find(childId) != mRelations.end())
        {
            continue;
        }
        auto impl = std::make_shared<model::CameraDirectoryRelationImpl>(
            parentId, childId, relation->getRelationType());
        mRelations.emplace(childId, impl);
        accepted.push_back(impl);
    }
    return accepted;
}

model::CameraDirectoryRelationArray CameraDirectoryModel::updateCameraRelationsInMemory(const model::CameraDirectoryRelationArray& relations)
{
    model::CameraDirectoryRelationArray accepted;
    accepted.reserve(relations.size());
    std::scoped_lock lock(mMutex);
    for (const auto& relation : relations)
    {
        if (!relation)
        {
            continue;
        }
        auto it = mRelations.find(relation->getChildId());
        if (it == mRelations.end())
        {
            continue;
        }
        it->second->setParentId(relation->getParentId());
        it->second->setRelationType(relation->getRelationType());
        accepted.push_back(it->second);
    }
    return accepted;
}

std::vector<std::string> CameraDirectoryModel::removeCameraRelationsInMemory(const std::vector<std::string>& childIds)
{
    std::vector<std::string> accepted;
    accepted.reserve(childIds.size());
    std::scoped_lock lock(mMutex);
    for (const auto& childId : childIds)
    {
        if (mRelations.erase(childId) > 0)
        {
            accepted.push_back(childId);
        }
    }
    return accepted;
}

// ===== Public write: memory first, then persist =====

model::CameraGroupArray CameraDirectoryModel::addCameraGroups(const model::CameraGroupArray& groups)
{
    auto accepted = addCameraGroupsInMemory(groups);
    if (!accepted.empty())
    {
        mCameraDirectoryDBAccess->insertCameraGroups(accepted);
    }
    return accepted;
}

model::CameraGroupArray CameraDirectoryModel::updateCameraGroups(const model::CameraGroupArray& groups)
{
    auto accepted = updateCameraGroupsInMemory(groups);
    for (const auto& g : accepted)
    {
        mCameraDirectoryDBAccess->updateCameraGroup(g);
    }
    return accepted;
}

std::vector<std::string> CameraDirectoryModel::removeCameraGroups(const std::vector<std::string>& nodeIds)
{
    auto accepted = removeCameraGroupsInMemory(nodeIds);
    for (const auto& nodeId : accepted)
    {
        mCameraDirectoryDBAccess->deleteCameraGroup(nodeId);
    }
    return accepted;
}

model::CameraEntryArray CameraDirectoryModel::addCameras(const model::CameraEntryArray& cameras)
{
    auto accepted = addCamerasInMemory(cameras);
    if (!accepted.empty())
    {
        mCameraDirectoryDBAccess->insertCameras(accepted);
    }
    return accepted;
}

model::CameraEntryArray CameraDirectoryModel::updateCameras(const model::CameraEntryArray& cameras)
{
    auto accepted = updateCamerasInMemory(cameras);
    for (const auto& e : accepted)
    {
        mCameraDirectoryDBAccess->updateCamera(e);
    }
    return accepted;
}

std::vector<std::string> CameraDirectoryModel::removeCameras(const std::vector<std::string>& nodeIds)
{
    auto accepted = removeCamerasInMemory(nodeIds);
    for (const auto& nodeId : accepted)
    {
        mCameraDirectoryDBAccess->deleteCamera(nodeId);
    }
    return accepted;
}

model::CameraDirectoryRelationArray CameraDirectoryModel::addCameraRelations(const model::CameraDirectoryRelationArray& relations)
{
    auto accepted = addCameraRelationsInMemory(relations);
    if (!accepted.empty())
    {
        mCameraDirectoryDBAccess->insertCameraRelations(accepted);
    }
    return accepted;
}

model::CameraDirectoryRelationArray CameraDirectoryModel::updateCameraRelations(const model::CameraDirectoryRelationArray& relations)
{
    auto accepted = updateCameraRelationsInMemory(relations);
    for (const auto& r : accepted)
    {
        mCameraDirectoryDBAccess->updateCameraRelation(r);
    }
    return accepted;
}

std::vector<std::string> CameraDirectoryModel::removeCameraRelations(const std::vector<std::string>& childIds)
{
    auto accepted = removeCameraRelationsInMemory(childIds);
    for (const auto& childId : accepted)
    {
        mCameraDirectoryDBAccess->deleteCameraRelation(childId);
    }
    return accepted;
}

// ===== Lifecycle =====

void CameraDirectoryModel::onDatabaseReady(const std::string& databaseId)
{
    SERVICE_LOG_DEBUG("Database ready, databaseId:" << databaseId);
    mCameraDirectoryDBAccess->setDatabaseId(databaseId);

    mCameraDirectoryDBAccess->loadCameraGroups(
        [this](const model::CameraGroupArray& groups)
        {
            addCameraGroupsInMemory(groups);
        });
    mCameraDirectoryDBAccess->loadCameras(
        [this](const model::CameraEntryArray& cameras)
        {
            addCamerasInMemory(cameras);
        });
    mCameraDirectoryDBAccess->loadCameraRelations(
        [this](const model::CameraDirectoryRelationArray& relations)
        {
            addCameraRelationsInMemory(relations);
        });
}

} // namespace ucf::service
