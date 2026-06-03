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
        if (auto sink = mNotificationSink.lock())
        {
            sink->onGroupsAdded(accepted, CameraDirectoryNotificationSource::Local);
        }
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
    if (!accepted.empty())
    {
        if (auto sink = mNotificationSink.lock())
        {
            sink->onGroupsUpdated(accepted, CameraDirectoryNotificationSource::Local);
        }
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
    if (!accepted.empty())
    {
        if (auto sink = mNotificationSink.lock())
        {
            sink->onGroupsRemoved(accepted, CameraDirectoryNotificationSource::Local);
        }
    }
    return accepted;
}

model::CameraEntryArray CameraDirectoryModel::addCameras(const model::CameraEntryArray& cameras)
{
    auto accepted = addCamerasInMemory(cameras);
    if (!accepted.empty())
    {
        mCameraDirectoryDBAccess->insertCameras(accepted);
        if (auto sink = mNotificationSink.lock())
        {
            sink->onCamerasAdded(accepted, CameraDirectoryNotificationSource::Local);
        }
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
    if (!accepted.empty())
    {
        if (auto sink = mNotificationSink.lock())
        {
            sink->onCamerasUpdated(accepted, CameraDirectoryNotificationSource::Local);
        }
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
    if (!accepted.empty())
    {
        if (auto sink = mNotificationSink.lock())
        {
            sink->onCamerasRemoved(accepted, CameraDirectoryNotificationSource::Local);
        }
    }
    return accepted;
}

model::CameraDirectoryRelationArray CameraDirectoryModel::addCameraRelations(const model::CameraDirectoryRelationArray& relations)
{
    auto accepted = addCameraRelationsInMemory(relations);
    if (!accepted.empty())
    {
        mCameraDirectoryDBAccess->insertCameraRelations(accepted);
        if (auto sink = mNotificationSink.lock())
        {
            sink->onRelationsAdded(accepted, CameraDirectoryNotificationSource::Local);
        }
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
    if (!accepted.empty())
    {
        if (auto sink = mNotificationSink.lock())
        {
            sink->onRelationsUpdated(accepted, CameraDirectoryNotificationSource::Local);
        }
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
    if (!accepted.empty())
    {
        if (auto sink = mNotificationSink.lock())
        {
            sink->onRelationsRemoved(accepted, CameraDirectoryNotificationSource::Local);
        }
    }
    return accepted;
}

// ===== Lifecycle =====

void CameraDirectoryModel::setNotificationSink(std::weak_ptr<ICameraDirectoryNotificationSink> sink)
{
    mNotificationSink = std::move(sink);
}

void CameraDirectoryModel::bindDatabase(const std::string& databaseId)
{
    if (databaseId.empty())
    {
        SERVICE_LOG_ERROR("bindDatabase ignored: empty databaseId");
        return;
    }

    // Atomic Uninit -> DbBound. On failure (already bound) verify same dbId and
    // either noop or reject; rebinding to a different DB is unsupported.
    if (auto expected = LoadStage::Uninit; !mLoadStage.compare_exchange_strong(expected, LoadStage::DbBound))
    {
        const auto& current = mCameraDirectoryDBAccess->getDatabaseId();
        if (current != databaseId)
        {
            SERVICE_LOG_ERROR("bindDatabase rejected: rebinding to a different db is not supported"
                              << ", current:" << current
                              << ", incoming:" << databaseId);
        }
        else
        {
            SERVICE_LOG_DEBUG("bindDatabase noop, same databaseId:" << databaseId);
        }
        return;
    }

    mCameraDirectoryDBAccess->setDatabaseId(databaseId);
    SERVICE_LOG_DEBUG("Bind database, databaseId:" << databaseId);

    // Auto-promote a load that was requested before the DB was bound, so callers
    // can invoke loadCameraDirectory() in any order relative to bindDatabase().
    if (mLoadPending.exchange(false))
    {
        SERVICE_LOG_DEBUG("bindDatabase auto-promoting pending loadCameraDirectory");
        loadCameraDirectory();
    }
}

void CameraDirectoryModel::loadCameraDirectory()
{
    // State guard: only DbBound / Failed -> Loading is allowed.
    auto stage = mLoadStage.load();
    if (stage == LoadStage::Uninit)
    {
        // DB not bound yet: park the request. bindDatabase() will auto-promote it
        // once the DB id has been set, so callers do not need to retry.
        mLoadPending.store(true);
        SERVICE_LOG_DEBUG("loadCameraDirectory deferred: database not bound yet, pending=true");
        return;
    }
    if (stage == LoadStage::Loading || stage == LoadStage::Ready)
    {
        SERVICE_LOG_DEBUG("loadCameraDirectory ignored: already loading or ready");
        return;
    }
    // DbBound or Failed -> Loading
    if (!mLoadStage.compare_exchange_strong(stage, LoadStage::Loading))
    {
        // Race: another thread already advanced the state; let them drive completion.
        return;
    }

    // Shared context: each chunk callback decrements the counter; the last one fires the
    // completion notification.
    struct LoadContext
    {
        std::atomic<int> remaining{3};
    };
    auto ctx = std::make_shared<LoadContext>();

    auto onChunkDone = [this, ctx]()
    {
        if (ctx->remaining.fetch_sub(1) == 1)
        {
            // Today's DBAccess interface does not distinguish "empty table" from "failure",
            // so we always finish as success here. Once DBAccess supports onError, switch
            // this path to finishLoadFailure(CameraDirectoryLoadError::DatabaseReadFailed).
            finishLoadSuccess();
        }
    };

    mCameraDirectoryDBAccess->loadCameraGroups(
        [this, onChunkDone](const model::CameraGroupArray& groups)
        {
            addCameraGroupsInMemory(groups);
            onChunkDone();
        });
    mCameraDirectoryDBAccess->loadCameras(
        [this, onChunkDone](const model::CameraEntryArray& cameras)
        {
            addCamerasInMemory(cameras);
            onChunkDone();
        });
    mCameraDirectoryDBAccess->loadCameraRelations(
        [this, onChunkDone](const model::CameraDirectoryRelationArray& relations)
        {
            addCameraRelationsInMemory(relations);
            onChunkDone();
        });
}

bool CameraDirectoryModel::isCameraDirectoryReady() const
{
    return mLoadStage.load() == LoadStage::Ready;
}

void CameraDirectoryModel::finishLoadSuccess()
{
    // CAS so a racing finishLoadFailure cannot also fire.
    if (auto expected = LoadStage::Loading; !mLoadStage.compare_exchange_strong(expected, LoadStage::Ready))
    {
        SERVICE_LOG_DEBUG("finishLoadSuccess skipped, stage already:" << static_cast<int>(expected));
        return;
    }
    SERVICE_LOG_DEBUG("loadCameraDirectory finished, success:true");
    if (auto sink = mNotificationSink.lock())
    {
        sink->onDirectoryLoaded();
    }
}

void CameraDirectoryModel::finishLoadFailure(CameraDirectoryLoadError error)
{
    if (auto expected = LoadStage::Loading; !mLoadStage.compare_exchange_strong(expected, LoadStage::Failed))
    {
        SERVICE_LOG_DEBUG("finishLoadFailure skipped, stage already:" << static_cast<int>(expected));
        return;
    }
    SERVICE_LOG_ERROR("loadCameraDirectory finished, success:false, error:" << static_cast<int>(error));
    if (auto sink = mNotificationSink.lock())
    {
        sink->onDirectoryLoadFailed(error);
    }
}

} // namespace ucf::service
