#include "CameraDirectoryViewModel.h"

#include <algorithm>
#include <utility>

#include <ucf/Services/CameraDirectoryService/ICameraDirectoryService.h>
#include <ucf/Services/CameraDirectoryService/ICameraDirectoryEntities.h>
#include <ucf/Services/MediaService/MediaTypes.h>

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/ServiceLocator/IServiceLocator.h>

#include "CameraDirectoryViewModelUtils.h"

namespace commonHead::viewModels {

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

std::shared_ptr<model::CameraDirectoryTree> CameraDirectoryViewModel::snapshotTree() const
{
    std::scoped_lock lk(mTreeMutex);
    return mTree;
}

model::CameraDirectoryTreeNodePtr CameraDirectoryViewModel::findNode(
    const std::shared_ptr<model::CameraDirectoryTree>& tree, const std::string& id) const
{
    if (!tree)
    {
        return nullptr;
    }
    return tree->findNodeById(id);
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
    return utils::toVMSource(entry->getSource());
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

// ===== Create / remove permissions =====

bool CameraDirectoryViewModel::canAddCameraNode(const std::string& parentId,
                                                model::CameraDirectoryNodeType /*type*/) const
{
    // Root is always a valid parent for either node type.
    if (parentId.empty())
    {
        return true;
    }
    auto parent = findNode(snapshotTree(), parentId);
    if (!parent)
    {
        return false;
    }
    // Only group nodes can hold children.
    return parent->getNodeData().type == model::CameraDirectoryNodeType::Group;
}

bool CameraDirectoryViewModel::canRemoveCameraNode(const std::string& nodeId) const
{
    if (nodeId.empty())
    {
        return false;
    }
    return findNode(snapshotTree(), nodeId) != nullptr;
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
        std::make_shared<utils::VMCameraGroup>(nodeId, displayName)
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
        std::make_shared<utils::VMCameraGroup>(nodeId, displayName)
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
        std::make_shared<utils::VMCameraEntry>(
            nodeId, displayName,
            utils::toServiceSource(source))
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
        std::make_shared<utils::VMCameraEntry>(
            nodeId, displayName,
            utils::toServiceSource(source))
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
    // relationId left empty: the service mints a UUID for the brand-new row.
    ucf::service::model::CameraDirectoryRelationArray relations{
        std::make_shared<utils::VMCameraDirectoryRelation>(std::string{}, parentId, childId)
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
    // The service identifies relation rows by RELATION_ID; look up the surrogate key
    // that the tree already remembers for this child.
    std::string relationId;
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree)
        {
            relationId = mTree->getRelationIdByChildId(childId);
        }
    }
    if (relationId.empty())
    {
        COMMONHEAD_LOG_WARN("updateRelation: no existing relation for childId:" << childId
                            << ", routing to addRelation instead");
        addRelation(parentId, childId);
        return;
    }
    COMMONHEAD_LOG_DEBUG("updateRelation, relationId:" << relationId
                         << ", parentId:" << parentId << ", childId:" << childId);
    ucf::service::model::CameraDirectoryRelationArray relations{
        std::make_shared<utils::VMCameraDirectoryRelation>(relationId, parentId, childId)
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
    // Translate childIds known to this VM into the RELATION_IDs the service expects.
    // Unknown childIds (already at root, or owned by a sibling VM) are silently skipped.
    std::vector<std::string> relationIds;
    relationIds.reserve(childIds.size());
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree)
        {
            for (const auto& childId : childIds)
            {
                auto rid = mTree->getRelationIdByChildId(childId);
                if (!rid.empty())
                {
                    relationIds.push_back(std::move(rid));
                }
            }
        }
    }
    COMMONHEAD_LOG_DEBUG("removeRelations, childIds:" << childIds.size()
                         << ", resolved relationIds:" << relationIds.size());
    if (relationIds.empty())
    {
        return;
    }
    service->removeCameraRelations(relationIds);
}

// ===== Drag-drop / re-parent =====

bool CameraDirectoryViewModel::canMoveCameraNode(const std::string& childId,
                                                 const std::string& newParentId) const
{
    if (childId.empty() || childId == newParentId)
    {
        return false;
    }
    auto snapshot = snapshotTree();
    auto child = findNode(snapshot, childId);
    if (!child)
    {
        return false;
    }
    // Empty newParentId = move to root, always allowed for any known node.
    if (!newParentId.empty())
    {
        auto parent = findNode(snapshot, newParentId);
        if (!parent)
        {
            return false;
        }
        // Only groups can contain other nodes.
        if (parent->getNodeData().type != model::CameraDirectoryNodeType::Group)
        {
            return false;
        }
        // Reject cycle: newParent must not be a descendant of child.
        if (utils::isAncestorOf(snapshot, childId, newParentId))
        {
            return false;
        }
    }
    // No-op move (already at requested parent) -> reject so caller doesn't issue a
    // wasted update round-trip.
    auto currentParent = child->getParent().lock();
    const std::string currentParentId = currentParent ? currentParent->getNodeData().id : std::string{};
    if (currentParentId == newParentId)
    {
        return false;
    }
    return true;
}

void CameraDirectoryViewModel::moveCameraNode(const std::string& childId, const std::string& newParentId)
{
    if (!canMoveCameraNode(childId, newParentId))
    {
        COMMONHEAD_LOG_DEBUG("moveCameraNode rejected, childId:" << childId
                             << ", newParentId:" << (newParentId.empty() ? "<root>" : newParentId));
        return;
    }
    // Decide which write primitive to use based on current state.
    std::string currentRelationId;
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree)
        {
            currentRelationId = mTree->getRelationIdByChildId(childId);
        }
    }
    COMMONHEAD_LOG_INFO("moveCameraNode, childId:" << childId
                        << ", newParentId:" << (newParentId.empty() ? "<root>" : newParentId)
                        << ", existingRelationId:" << (currentRelationId.empty() ? "<none>" : currentRelationId));
    if (newParentId.empty())
    {
        // Move to root = drop the relation row entirely. removeRelations does the
        // child->relationId translation internally.
        removeRelations({childId});
        return;
    }
    if (currentRelationId.empty())
    {
        // Currently at root, no relation row yet -> add one.
        addRelation(newParentId, childId);
    }
    else
    {
        // Re-parent the existing relation row.
        updateRelation(newParentId, childId);
    }
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
    fireNotification(&ICameraDirectoryViewModelCallback::onCameraDirectoryLoadFailed, utils::toVMLoadError(error));
}

void CameraDirectoryViewModel::onCameraGroupsAdded(const ucf::service::model::CameraGroupArray& groups)
{
    auto vmGroups = utils::toVMNodeDatas(groups);
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
    auto vmGroups = utils::toVMNodeDatas(groups);
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
    auto vmCameras = utils::toVMNodeDatas(cameras);
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
    auto vmCameras = utils::toVMNodeDatas(cameras);
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
    auto vmRelations = utils::toVMRelations(relations);
    if (ensureTreeBuilt())
    {
        COMMONHEAD_LOG_WARN("onCameraRelationsAdded arrived before tree was built (count:" << vmRelations.size()
                            << "); fired onCameraDirectoryReady instead of delta");
        fireNotification(&ICameraDirectoryViewModelCallback::onCameraDirectoryReady);
        return;
    }
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree) mTree->setRelations(vmRelations);
    }
    fireNotification(&ICameraDirectoryViewModelCallback::onCameraRelationsAdded, vmRelations);
}

void CameraDirectoryViewModel::onCameraRelationsUpdated(const ucf::service::model::CameraDirectoryRelationArray& relations)
{
    auto vmRelations = utils::toVMRelations(relations);
    if (ensureTreeBuilt())
    {
        COMMONHEAD_LOG_WARN("onCameraRelationsUpdated arrived before tree was built (count:" << vmRelations.size()
                            << "); fired onCameraDirectoryReady instead of delta");
        fireNotification(&ICameraDirectoryViewModelCallback::onCameraDirectoryReady);
        return;
    }
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree) mTree->setRelations(vmRelations);
    }
    fireNotification(&ICameraDirectoryViewModelCallback::onCameraRelationsUpdated, vmRelations);
}

void CameraDirectoryViewModel::onCameraRelationsRemoved(const std::vector<std::string>& relationIds)
{
    if (ensureTreeBuilt())
    {
        COMMONHEAD_LOG_WARN("onCameraRelationsRemoved arrived before tree was built (count:" << relationIds.size()
                            << "); fired onCameraDirectoryReady instead of delta");
        fireNotification(&ICameraDirectoryViewModelCallback::onCameraDirectoryReady);
        return;
    }
    // Service emits a flat list of relationIds. Translate each back to the childId that
    // belongs to this VM (unknown ids are silently dropped) so subscribers continue to
    // work in their familiar childId vocabulary.
    std::vector<std::string> childIds;
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree)
        {
            childIds.reserve(relationIds.size());
            for (const auto& rid : relationIds)
            {
                auto childId = mTree->takeChildIdByRelationId(rid);
                if (!childId.empty())
                {
                    childIds.push_back(std::move(childId));
                }
            }
            mTree->clearRelations(childIds);
        }
    }
    if (childIds.empty())
    {
        return;
    }
    fireNotification(&ICameraDirectoryViewModelCallback::onCameraRelationsRemoved, childIds);
}

}
