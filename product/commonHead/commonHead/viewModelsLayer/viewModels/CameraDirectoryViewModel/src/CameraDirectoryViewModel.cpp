#include "CameraDirectoryViewModel.h"

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
    if (auto service = lockService())
    {
        service->registerCallback(
            std::static_pointer_cast<ucf::service::ICameraDirectoryServiceCallback>(shared_from_this()));
    }
    rebuildTreeFromService();
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
        return;
    }
    auto groups    = service->getCameraGroups();
    auto cameras   = service->getCameras();
    auto relations = service->getCameraRelations();

    auto tree = std::make_shared<model::CameraDirectoryTree>(groups, cameras, relations);
    {
        std::scoped_lock lk(mTreeMutex);
        mTree = std::move(tree);
    }
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

// ===== Write: Groups =====

void CameraDirectoryViewModel::addCameraGroup(const std::string& nodeId, const std::string& displayName)
{
    auto service = lockService();
    if (!service)
    {
        return;
    }
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
        return;
    }
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
        return;
    }
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
        return;
    }
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
        return;
    }
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
        return;
    }
    service->removeCameras(nodeIds);
}

// ===== Write: Relations =====

void CameraDirectoryViewModel::addRelation(const std::string& parentId, const std::string& childId)
{
    auto service = lockService();
    if (!service)
    {
        return;
    }
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
        return;
    }
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
        return;
    }
    service->removeCameraRelations(childIds);
}

// ===== ICameraDirectoryServiceCallback：增量回放并转发 =====

void CameraDirectoryViewModel::onCameraDirectoryReady()
{
    rebuildTreeFromService();
    fireNotification(&ICameraDirectoryViewModelCallback::onCameraDirectoryReady);
}

void CameraDirectoryViewModel::onCameraGroupsAdded(const ucf::service::model::CameraGroupArray& groups)
{
    auto vmGroups = toVMNodeDatas(groups);
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree) mTree->addNodes(vmGroups);
    }
    fireNotification(&ICameraDirectoryViewModelCallback::onCameraGroupsAdded, vmGroups);
}

void CameraDirectoryViewModel::onCameraGroupsUpdated(const ucf::service::model::CameraGroupArray& groups)
{
    auto vmGroups = toVMNodeDatas(groups);
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree) mTree->updateNodes(vmGroups);
    }
    fireNotification(&ICameraDirectoryViewModelCallback::onCameraGroupsUpdated, vmGroups);
}

void CameraDirectoryViewModel::onCameraGroupsRemoved(const std::vector<std::string>& nodeIds)
{
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree) mTree->removeNodes(nodeIds);
    }
    fireNotification(&ICameraDirectoryViewModelCallback::onCameraGroupsRemoved, nodeIds);
}

void CameraDirectoryViewModel::onCamerasAdded(const ucf::service::model::CameraEntryArray& cameras)
{
    auto vmCameras = toVMNodeDatas(cameras);
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree) mTree->addNodes(vmCameras);
    }
    fireNotification(&ICameraDirectoryViewModelCallback::onCamerasAdded, vmCameras);
}

void CameraDirectoryViewModel::onCamerasUpdated(const ucf::service::model::CameraEntryArray& cameras)
{
    auto vmCameras = toVMNodeDatas(cameras);
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree) mTree->updateNodes(vmCameras);
    }
    fireNotification(&ICameraDirectoryViewModelCallback::onCamerasUpdated, vmCameras);
}

void CameraDirectoryViewModel::onCamerasRemoved(const std::vector<std::string>& nodeIds)
{
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree) mTree->removeNodes(nodeIds);
    }
    fireNotification(&ICameraDirectoryViewModelCallback::onCamerasRemoved, nodeIds);
}

void CameraDirectoryViewModel::onCameraRelationsAdded(const ucf::service::model::CameraDirectoryRelationArray& relations)
{
    auto vmRelations = toVMRelations(relations);
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
    {
        std::scoped_lock lk(mTreeMutex);
        if (mTree) mTree->clearRelations(childIds);
    }
    fireNotification(&ICameraDirectoryViewModelCallback::onCameraRelationsRemoved, childIds);
}

}
