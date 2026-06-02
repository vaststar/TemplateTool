#include "CameraDirectoryViewModel.h"

#include <algorithm>
#include <utility>
#include <variant>

#include <ucf/Services/CameraDirectoryService/ICameraDirectoryService.h>
#include <ucf/Services/CameraDirectoryService/ICameraDirectoryEntities.h>
#include <ucf/Services/MediaService/MediaTypes.h>

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/ServiceLocator/IServiceLocator.h>

namespace commonHead::viewModels {

namespace {

ucf::service::media::CameraSource toServiceSource(const model::CameraSource& vm)
{
    return std::visit([](auto&& s) -> ucf::service::media::CameraSource {
        using T = std::decay_t<decltype(s)>;
        if constexpr (std::is_same_v<T, model::LocalCameraSource>)
        {
            return ucf::service::media::LocalCameraSource{s.index};
        }
        else
        {
            return ucf::service::media::NetworkCameraSource{
                s.url, s.transport, s.openTimeoutMs, s.readTimeoutMs};
        }
    }, vm);
}

model::CameraSource toVMSource(const ucf::service::media::CameraSource& svc)
{
    return std::visit([](auto&& s) -> model::CameraSource {
        using T = std::decay_t<decltype(s)>;
        if constexpr (std::is_same_v<T, ucf::service::media::LocalCameraSource>)
        {
            return model::LocalCameraSource{s.index};
        }
        else
        {
            return model::NetworkCameraSource{
                s.url, s.transport, s.openTimeoutMs, s.readTimeoutMs};
        }
    }, svc);
}

model::CameraNodeStatus toVMStatus(ucf::service::model::CameraNodeStatus s)
{
    return static_cast<model::CameraNodeStatus>(static_cast<int>(s));
}

model::CameraDirectoryLoadError toVMLoadError(ucf::service::CameraDirectoryLoadError e)
{
    switch (e)
    {
    case ucf::service::CameraDirectoryLoadError::DatabaseNotBound:
        return model::CameraDirectoryLoadError::DatabaseNotBound;
    case ucf::service::CameraDirectoryLoadError::DatabaseReadFailed:
        return model::CameraDirectoryLoadError::DatabaseReadFailed;
    case ucf::service::CameraDirectoryLoadError::Unknown:
    default:
        return model::CameraDirectoryLoadError::Unknown;
    }
}

model::CameraDirectoryNodeData toVMNodeData(const ucf::service::model::ICameraGroupPtr& g)
{
    model::CameraDirectoryNodeData d;
    if (g)
    {
        d.id          = g->getNodeId();
        d.displayName = g->getDisplayName();
        d.type        = model::CameraDirectoryNodeType::Group;
        d.status      = toVMStatus(g->getNodeStatus());
    }
    return d;
}

model::CameraDirectoryNodeData toVMNodeData(const ucf::service::model::ICameraEntryPtr& c)
{
    model::CameraDirectoryNodeData d;
    if (c)
    {
        d.id          = c->getNodeId();
        d.displayName = c->getDisplayName();
        d.type        = model::CameraDirectoryNodeType::Camera;
        d.status      = toVMStatus(c->getNodeStatus());
    }
    return d;
}

model::CameraDirectoryRelationData toVMRelation(const ucf::service::model::ICameraDirectoryRelationPtr& r)
{
    model::CameraDirectoryRelationData d;
    if (r)
    {
        d.parentId = r->getParentId();
        d.childId  = r->getChildId();
    }
    return d;
}

std::vector<model::CameraDirectoryNodeData> toVMNodeDatas(const ucf::service::model::CameraGroupArray& groups)
{
    std::vector<model::CameraDirectoryNodeData> out;
    out.reserve(groups.size());
    for (const auto& g : groups)
    {
        if (g)
        {
            out.push_back(toVMNodeData(g));
        }
    }
    return out;
}

std::vector<model::CameraDirectoryNodeData> toVMNodeDatas(const ucf::service::model::CameraEntryArray& cameras)
{
    std::vector<model::CameraDirectoryNodeData> out;
    out.reserve(cameras.size());
    for (const auto& c : cameras)
    {
        if (c)
        {
            out.push_back(toVMNodeData(c));
        }
    }
    return out;
}

std::vector<model::CameraDirectoryRelationData> toVMRelations(const ucf::service::model::CameraDirectoryRelationArray& relations)
{
    std::vector<model::CameraDirectoryRelationData> out;
    out.reserve(relations.size());
    for (const auto& r : relations)
    {
        if (r)
        {
            out.push_back(toVMRelation(r));
        }
    }
    return out;
}

}

std::shared_ptr<ICameraDirectoryViewModel> ICameraDirectoryViewModel::createInstance(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<CameraDirectoryViewModel>(commonHeadFramework);
}

CameraDirectoryViewModel::CameraDirectoryViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
    : ICameraDirectoryViewModel(commonHeadFramework)
{
    COMMONHEAD_LOG_DEBUG("create CameraDirectoryViewModel");
}

std::string CameraDirectoryViewModel::getViewModelName() const
{
    return "CameraDirectoryViewModel";
}

void CameraDirectoryViewModel::init()
{
    auto service = lockService();
    if (!service)
    {
        COMMONHEAD_LOG_ERROR("CameraDirectoryViewModel init: service not available");
        return;
    }
    // Register first so we do not miss any change event that arrives between the
    // isCameraDirectoryReady() probe and the load decision below.
    service->registerCallback(
        std::static_pointer_cast<ucf::service::ICameraDirectoryServiceCallback>(shared_from_this()));

    if (service->isCameraDirectoryReady())
    {
        // Late subscriber: the service already finished its load and will not re-fire
        // onCameraDirectoryReady, so build the tree synchronously from the current snapshot.
        COMMONHEAD_LOG_DEBUG("CameraDirectoryViewModel init: service already ready, rebuilding tree synchronously");
        rebuildTreeFromService();
        fireNotification(&ICameraDirectoryViewModelCallback::onCameraDirectoryReady);
    }
    else
    {
        // Trigger the load (idempotent if it is already in progress); the tree will be
        // built when onCameraDirectoryReady is delivered.
        COMMONHEAD_LOG_DEBUG("CameraDirectoryViewModel init: requesting service to load camera directory");
        service->loadCameraDirectory();
    }
}

std::shared_ptr<ucf::service::ICameraDirectoryService> CameraDirectoryViewModel::lockService() const
{
    if (auto framework = getCommonHeadFramework().lock())
    {
        if (auto locator = framework->getServiceLocator())
        {
            return locator->getCameraDirectoryService().lock();
        }
    }
    return nullptr;
}

void CameraDirectoryViewModel::rebuildTreeFromService()
{
    auto service = lockService();
    if (!service)
    {
        COMMONHEAD_LOG_ERROR("rebuildTreeFromService skipped: service not available");
        return;
    }
    auto groups    = service->getCameraGroups();
    auto cameras   = service->getCameras();
    auto relations = service->getCameraRelations();
    COMMONHEAD_LOG_DEBUG("rebuildTreeFromService, groups:" << groups.size()
                         << ", cameras:" << cameras.size()
                         << ", relations:" << relations.size());

    auto tree = std::make_shared<model::CameraDirectoryTree>(groups, cameras, relations);
    {
        std::scoped_lock lk(mTreeMutex);
        mTree = std::move(tree);
    }
}

bool CameraDirectoryViewModel::ensureTreeBuilt()
{
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree)
        {
            return false;
        }
    }
    // Out-of-order safety net: an incremental event arrived before our tree was built
    // (typically before onCameraDirectoryReady). Pull a full snapshot now so the VM state
    // is self-consistent; callers will surface this to subscribers as a Ready notification
    // rather than as the original delta, because the snapshot already contains the delta
    // plus everything else that was loaded before the subscriber attached.
    COMMONHEAD_LOG_DEBUG("ensureTreeBuilt: tree is null on incremental event, rebuilding full snapshot");
    rebuildTreeFromService();
    return true;
}

// ===== Read =====

model::CameraDirectoryTreePtr CameraDirectoryViewModel::getCameraTree() const
{
    std::scoped_lock lk(mTreeMutex);
    return std::static_pointer_cast<model::ICameraDirectoryTree>(mTree);
}

std::optional<model::CameraSource> CameraDirectoryViewModel::getCameraSource(const std::string& nodeId) const
{
    auto service = lockService();
    if (!service)
    {
        return std::nullopt;
    }
    auto entry = service->getCamera(nodeId);
    if (!entry)
    {
        return std::nullopt;
    }
    return toVMSource(entry->getSource());
}

bool CameraDirectoryViewModel::isCameraDirectoryReady() const
{
    std::scoped_lock lk(mTreeMutex);
    return static_cast<bool>(mTree);
}

std::string CameraDirectoryViewModel::getCurrentCameraId() const
{
    std::scoped_lock lk(mSelectionMutex);
    return mCurrentCameraId;
}

// ===== Selection =====

void CameraDirectoryViewModel::selectCamera(const std::string& nodeId)
{
    // Empty id is a valid "clear" command; non-empty must resolve to an actual camera.
    if (!nodeId.empty())
    {
        auto service = lockService();
        if (!service || !service->getCamera(nodeId))
        {
            COMMONHEAD_LOG_WARN("selectCamera ignored: not a known camera, nodeId:" << nodeId);
            return;
        }
    }

    {
        std::scoped_lock lk(mSelectionMutex);
        if (mCurrentCameraId == nodeId)
        {
            return;
        }
        mCurrentCameraId = nodeId;
    }
    // Single funnel for selection metrics / analytics hooks.
    COMMONHEAD_LOG_INFO("selectCamera: nodeId:" << (nodeId.empty() ? "<cleared>" : nodeId));
    fireNotification(&ICameraDirectoryViewModelCallback::onCurrentCameraChanged, nodeId);
}

// ===== Write: Groups =====

void CameraDirectoryViewModel::addCameraGroup(const std::string& nodeId, const std::string& displayName)
{
    auto service = lockService();
    if (!service)
    {
        COMMONHEAD_LOG_ERROR("addCameraGroup ignored: service not available, nodeId:" << nodeId);
        return;
    }
    COMMONHEAD_LOG_DEBUG("addCameraGroup, nodeId:" << nodeId << ", displayName:" << displayName);
    ucf::service::model::CameraGroupArray groups{
        ucf::service::model::ICameraGroup::createInstance(nodeId, displayName)
    };
    service->addCameraGroups(groups);
}

void CameraDirectoryViewModel::updateCameraGroup(const std::string& nodeId, const std::string& displayName)
{
    auto service = lockService();
    if (!service)
    {
        COMMONHEAD_LOG_ERROR("updateCameraGroup ignored: service not available, nodeId:" << nodeId);
        return;
    }
    COMMONHEAD_LOG_DEBUG("updateCameraGroup, nodeId:" << nodeId << ", displayName:" << displayName);
    ucf::service::model::CameraGroupArray groups{
        ucf::service::model::ICameraGroup::createInstance(nodeId, displayName)
    };
    service->updateCameraGroups(groups);
}

void CameraDirectoryViewModel::removeCameraGroups(const std::vector<std::string>& nodeIds)
{
    auto service = lockService();
    if (!service)
    {
        COMMONHEAD_LOG_ERROR("removeCameraGroups ignored: service not available, count:" << nodeIds.size());
        return;
    }
    COMMONHEAD_LOG_DEBUG("removeCameraGroups, count:" << nodeIds.size());
    service->removeCameraGroups(nodeIds);
}

// ===== Write: Cameras =====

void CameraDirectoryViewModel::addCamera(const std::string& nodeId,
                                         const std::string& displayName,
                                         const model::CameraSource& source)
{
    auto service = lockService();
    if (!service)
    {
        COMMONHEAD_LOG_ERROR("addCamera ignored: service not available, nodeId:" << nodeId);
        return;
    }
    COMMONHEAD_LOG_DEBUG("addCamera, nodeId:" << nodeId << ", displayName:" << displayName);
    ucf::service::model::CameraEntryArray cameras{
        ucf::service::model::ICameraEntry::createInstance(
            nodeId, displayName,
            toServiceSource(source))
    };
    service->addCameras(cameras);
}

void CameraDirectoryViewModel::updateCamera(const std::string& nodeId,
                                            const std::string& displayName,
                                            const model::CameraSource& source)
{
    auto service = lockService();
    if (!service)
    {
        COMMONHEAD_LOG_ERROR("updateCamera ignored: service not available, nodeId:" << nodeId);
        return;
    }
    COMMONHEAD_LOG_DEBUG("updateCamera, nodeId:" << nodeId << ", displayName:" << displayName);
    ucf::service::model::CameraEntryArray cameras{
        ucf::service::model::ICameraEntry::createInstance(
            nodeId, displayName,
            toServiceSource(source))
    };
    service->updateCameras(cameras);
}

void CameraDirectoryViewModel::removeCameras(const std::vector<std::string>& nodeIds)
{
    auto service = lockService();
    if (!service)
    {
        COMMONHEAD_LOG_ERROR("removeCameras ignored: service not available, count:" << nodeIds.size());
        return;
    }
    COMMONHEAD_LOG_DEBUG("removeCameras, count:" << nodeIds.size());
    service->removeCameras(nodeIds);
}

// ===== Write: Relations =====

void CameraDirectoryViewModel::addRelation(const std::string& parentId, const std::string& childId)
{
    auto service = lockService();
    if (!service)
    {
        COMMONHEAD_LOG_ERROR("addRelation ignored: service not available, childId:" << childId);
        return;
    }
    COMMONHEAD_LOG_DEBUG("addRelation, parentId:" << parentId << ", childId:" << childId);
    ucf::service::model::CameraDirectoryRelationArray relations{
        ucf::service::model::ICameraDirectoryRelation::createInstance(parentId, childId)
    };
    service->addCameraRelations(relations);
}

void CameraDirectoryViewModel::updateRelation(const std::string& parentId, const std::string& childId)
{
    auto service = lockService();
    if (!service)
    {
        COMMONHEAD_LOG_ERROR("updateRelation ignored: service not available, childId:" << childId);
        return;
    }
    COMMONHEAD_LOG_DEBUG("updateRelation, parentId:" << parentId << ", childId:" << childId);
    ucf::service::model::CameraDirectoryRelationArray relations{
        ucf::service::model::ICameraDirectoryRelation::createInstance(parentId, childId)
    };
    service->updateCameraRelations(relations);
}

void CameraDirectoryViewModel::removeRelations(const std::vector<std::string>& childIds)
{
    auto service = lockService();
    if (!service)
    {
        COMMONHEAD_LOG_ERROR("removeRelations ignored: service not available, count:" << childIds.size());
        return;
    }
    COMMONHEAD_LOG_DEBUG("removeRelations, count:" << childIds.size());
    service->removeCameraRelations(childIds);
}

// ===== ICameraDirectoryServiceCallback: apply incremental updates then forward to VM subscribers =====

void CameraDirectoryViewModel::onCameraDirectoryReady()
{
    COMMONHEAD_LOG_DEBUG("onCameraDirectoryReady received from service");
    rebuildTreeFromService();
    fireNotification(&ICameraDirectoryViewModelCallback::onCameraDirectoryReady);
}

void CameraDirectoryViewModel::onCameraDirectoryLoadFailed(ucf::service::CameraDirectoryLoadError error)
{
    COMMONHEAD_LOG_ERROR("onCameraDirectoryLoadFailed received from service, error:" << static_cast<int>(error));
    fireNotification(&ICameraDirectoryViewModelCallback::onCameraDirectoryLoadFailed, toVMLoadError(error));
}

void CameraDirectoryViewModel::onCameraGroupsAdded(const ucf::service::model::CameraGroupArray& groups)
{
    auto vmGroups = toVMNodeDatas(groups);
    if (ensureTreeBuilt())
    {
        // Snapshot rebuilt; the delta is already included. Surface as Ready so subscribers
        // re-sync from getCameraTree() instead of double-applying the delta.
        COMMONHEAD_LOG_WARN("onCameraGroupsAdded arrived before tree was built (count:" << vmGroups.size()
                            << "); fired onCameraDirectoryReady instead of delta");
        fireNotification(&ICameraDirectoryViewModelCallback::onCameraDirectoryReady);
        return;
    }
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree) mTree->addNodes(vmGroups);
    }
    fireNotification(&ICameraDirectoryViewModelCallback::onCameraGroupsAdded, vmGroups);
}

void CameraDirectoryViewModel::onCameraGroupsUpdated(const ucf::service::model::CameraGroupArray& groups)
{
    auto vmGroups = toVMNodeDatas(groups);
    if (ensureTreeBuilt())
    {
        COMMONHEAD_LOG_WARN("onCameraGroupsUpdated arrived before tree was built (count:" << vmGroups.size()
                            << "); fired onCameraDirectoryReady instead of delta");
        fireNotification(&ICameraDirectoryViewModelCallback::onCameraDirectoryReady);
        return;
    }
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree) mTree->updateNodes(vmGroups);
    }
    fireNotification(&ICameraDirectoryViewModelCallback::onCameraGroupsUpdated, vmGroups);
}

void CameraDirectoryViewModel::onCameraGroupsRemoved(const std::vector<std::string>& nodeIds)
{
    if (ensureTreeBuilt())
    {
        COMMONHEAD_LOG_WARN("onCameraGroupsRemoved arrived before tree was built (count:" << nodeIds.size()
                            << "); fired onCameraDirectoryReady instead of delta");
        fireNotification(&ICameraDirectoryViewModelCallback::onCameraDirectoryReady);
        return;
    }
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree) mTree->removeNodes(nodeIds);
    }
    fireNotification(&ICameraDirectoryViewModelCallback::onCameraGroupsRemoved, nodeIds);
}

void CameraDirectoryViewModel::onCamerasAdded(const ucf::service::model::CameraEntryArray& cameras)
{
    auto vmCameras = toVMNodeDatas(cameras);
    if (ensureTreeBuilt())
    {
        COMMONHEAD_LOG_WARN("onCamerasAdded arrived before tree was built (count:" << vmCameras.size()
                            << "); fired onCameraDirectoryReady instead of delta");
        fireNotification(&ICameraDirectoryViewModelCallback::onCameraDirectoryReady);
        return;
    }
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree) mTree->addNodes(vmCameras);
    }
    fireNotification(&ICameraDirectoryViewModelCallback::onCamerasAdded, vmCameras);
}

void CameraDirectoryViewModel::onCamerasUpdated(const ucf::service::model::CameraEntryArray& cameras)
{
    auto vmCameras = toVMNodeDatas(cameras);
    if (ensureTreeBuilt())
    {
        COMMONHEAD_LOG_WARN("onCamerasUpdated arrived before tree was built (count:" << vmCameras.size()
                            << "); fired onCameraDirectoryReady instead of delta");
        fireNotification(&ICameraDirectoryViewModelCallback::onCameraDirectoryReady);
        return;
    }
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree) mTree->updateNodes(vmCameras);
    }
    fireNotification(&ICameraDirectoryViewModelCallback::onCamerasUpdated, vmCameras);
}

void CameraDirectoryViewModel::onCamerasRemoved(const std::vector<std::string>& nodeIds)
{
    if (ensureTreeBuilt())
    {
        COMMONHEAD_LOG_WARN("onCamerasRemoved arrived before tree was built (count:" << nodeIds.size()
                            << "); fired onCameraDirectoryReady instead of delta");
        fireNotification(&ICameraDirectoryViewModelCallback::onCameraDirectoryReady);
        return;
    }
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree) mTree->removeNodes(nodeIds);
    }
    // If the currently selected camera was just removed, clear the selection so UI
    // mirrors do not keep stale ids highlighted.
    bool clearedSelection = false;
    {
        std::scoped_lock lk(mSelectionMutex);
        if (!mCurrentCameraId.empty()
            && std::find(nodeIds.begin(), nodeIds.end(), mCurrentCameraId) != nodeIds.end())
        {
            mCurrentCameraId.clear();
            clearedSelection = true;
        }
    }
    fireNotification(&ICameraDirectoryViewModelCallback::onCamerasRemoved, nodeIds);
    if (clearedSelection)
    {
        COMMONHEAD_LOG_INFO("selectCamera auto-cleared: selected camera was removed");
        fireNotification(&ICameraDirectoryViewModelCallback::onCurrentCameraChanged, std::string{});
    }
}

void CameraDirectoryViewModel::onCameraRelationsAdded(const ucf::service::model::CameraDirectoryRelationArray& relations)
{
    auto vmRelations = toVMRelations(relations);
    if (ensureTreeBuilt())
    {
        COMMONHEAD_LOG_WARN("onCameraRelationsAdded arrived before tree was built (count:" << vmRelations.size()
                            << "); fired onCameraDirectoryReady instead of delta");
        fireNotification(&ICameraDirectoryViewModelCallback::onCameraDirectoryReady);
        return;
    }
    std::vector<std::pair<std::string, std::string>> pairs;
    pairs.reserve(vmRelations.size());
    for (const auto& r : vmRelations) pairs.emplace_back(r.parentId, r.childId);
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree) mTree->setRelations(pairs);
    }
    fireNotification(&ICameraDirectoryViewModelCallback::onCameraRelationsAdded, vmRelations);
}

void CameraDirectoryViewModel::onCameraRelationsUpdated(const ucf::service::model::CameraDirectoryRelationArray& relations)
{
    auto vmRelations = toVMRelations(relations);
    if (ensureTreeBuilt())
    {
        COMMONHEAD_LOG_WARN("onCameraRelationsUpdated arrived before tree was built (count:" << vmRelations.size()
                            << "); fired onCameraDirectoryReady instead of delta");
        fireNotification(&ICameraDirectoryViewModelCallback::onCameraDirectoryReady);
        return;
    }
    std::vector<std::pair<std::string, std::string>> pairs;
    pairs.reserve(vmRelations.size());
    for (const auto& r : vmRelations) pairs.emplace_back(r.parentId, r.childId);
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree) mTree->setRelations(pairs);
    }
    fireNotification(&ICameraDirectoryViewModelCallback::onCameraRelationsUpdated, vmRelations);
}

void CameraDirectoryViewModel::onCameraRelationsRemoved(const std::vector<std::string>& childIds)
{
    if (ensureTreeBuilt())
    {
        COMMONHEAD_LOG_WARN("onCameraRelationsRemoved arrived before tree was built (count:" << childIds.size()
                            << "); fired onCameraDirectoryReady instead of delta");
        fireNotification(&ICameraDirectoryViewModelCallback::onCameraDirectoryReady);
        return;
    }
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree) mTree->clearRelations(childIds);
    }
    fireNotification(&ICameraDirectoryViewModelCallback::onCameraRelationsRemoved, childIds);
}

}
