#include "CameraDirectoryViewModelUtils.h"

#include <type_traits>
#include <utility>
#include <variant>

#include "CameraDirectoryTreeModel.h"

namespace commonHead::viewModels::utils {

// ===== Source / status / load-error converters =====

ucf::service::media::CameraSource toServiceSource(const model::CameraSource& vmSource)
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
    }, vmSource);
}

model::CameraSource toVMSource(const ucf::service::media::CameraSource& serviceSource)
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
    }, serviceSource);
}

model::CameraNodeStatus toVMStatus(ucf::service::model::CameraNodeStatus serviceStatus)
{
    return static_cast<model::CameraNodeStatus>(static_cast<int>(serviceStatus));
}

model::CameraDirectoryLoadError toVMLoadError(ucf::service::CameraDirectoryLoadError serviceError)
{
    switch (serviceError)
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

// ===== Service-entity -> VM-data converters =====

model::CameraDirectoryNodeData toVMNodeData(const ucf::service::model::ICameraGroupPtr& group)
{
    model::CameraDirectoryNodeData nodeData;
    if (group)
    {
        nodeData.id          = group->getNodeId();
        nodeData.displayName = group->getDisplayName();
        nodeData.type        = model::CameraDirectoryNodeType::Group;
        nodeData.status      = toVMStatus(group->getNodeStatus());
    }
    return nodeData;
}

model::CameraDirectoryNodeData toVMNodeData(const ucf::service::model::ICameraEntryPtr& camera)
{
    model::CameraDirectoryNodeData nodeData;
    if (camera)
    {
        nodeData.id          = camera->getNodeId();
        nodeData.displayName = camera->getDisplayName();
        nodeData.type        = model::CameraDirectoryNodeType::Camera;
        nodeData.status      = toVMStatus(camera->getNodeStatus());
    }
    return nodeData;
}

model::CameraDirectoryRelationData toVMRelation(const ucf::service::model::ICameraDirectoryRelationPtr& serviceRelation)
{
    model::CameraDirectoryRelationData relationData;
    if (serviceRelation)
    {
        relationData.id       = serviceRelation->getRelationId();
        relationData.parentId = serviceRelation->getParentId();
        relationData.childId  = serviceRelation->getChildId();
    }
    return relationData;
}

std::vector<model::CameraDirectoryNodeData> toVMNodeDatas(const ucf::service::model::CameraGroupArray& groups)
{
    std::vector<model::CameraDirectoryNodeData> out;
    out.reserve(groups.size());
    for (const auto& group : groups)
    {
        if (group)
        {
            out.push_back(toVMNodeData(group));
        }
    }
    return out;
}

std::vector<model::CameraDirectoryNodeData> toVMNodeDatas(const ucf::service::model::CameraEntryArray& cameras)
{
    std::vector<model::CameraDirectoryNodeData> out;
    out.reserve(cameras.size());
    for (const auto& camera : cameras)
    {
        if (camera)
        {
            out.push_back(toVMNodeData(camera));
        }
    }
    return out;
}

std::vector<model::CameraDirectoryRelationData> toVMRelations(const ucf::service::model::CameraDirectoryRelationArray& serviceRelations)
{
    std::vector<model::CameraDirectoryRelationData> out;
    out.reserve(serviceRelations.size());
    for (const auto& relation : serviceRelations)
    {
        if (relation)
        {
            out.push_back(toVMRelation(relation));
        }
    }
    return out;
}

// ===== Tree helpers =====

bool isAncestorOf(const std::shared_ptr<model::CameraDirectoryTree>& tree,
                  const std::string& ancestorCandidateId,
                  const std::string& startNodeId)
{
    if (!tree || ancestorCandidateId.empty() || startNodeId.empty())
    {
        return false;
    }
    auto node = tree->findNodeById(startNodeId);
    if (!node)
    {
        return false;
    }
    auto parent = node->getParent().lock();
    while (parent)
    {
        const auto parentId = parent->getNodeData().id;
        if (parentId == ancestorCandidateId)
        {
            return true;
        }
        if (parentId.empty())
        {
            break; // hit the virtual root
        }
        parent = parent->getParent().lock();
    }
    return false;
}

// ===== Minimal ucf::service::model::* impls owned by the VM =====

VMCameraGroup::VMCameraGroup(std::string nodeId,
                             std::string displayName,
                             ucf::service::model::CameraNodeStatus status)
    : mNodeId(std::move(nodeId))
    , mDisplayName(std::move(displayName))
    , mStatus(status)
{
}

std::string                            VMCameraGroup::getNodeId()      const { return mNodeId; }
std::string                            VMCameraGroup::getDisplayName() const { return mDisplayName; }
ucf::service::model::CameraNodeStatus  VMCameraGroup::getNodeStatus()  const { return mStatus; }

VMCameraEntry::VMCameraEntry(std::string nodeId,
                             std::string displayName,
                             ucf::service::media::CameraSource source,
                             ucf::service::model::CameraNodeStatus status)
    : mNodeId(std::move(nodeId))
    , mDisplayName(std::move(displayName))
    , mStatus(status)
    , mSource(std::move(source))
{
}

std::string                            VMCameraEntry::getNodeId()      const { return mNodeId; }
std::string                            VMCameraEntry::getDisplayName() const { return mDisplayName; }
ucf::service::model::CameraNodeStatus  VMCameraEntry::getNodeStatus()  const { return mStatus; }
ucf::service::media::CameraSource      VMCameraEntry::getSource()      const { return mSource; }

VMCameraDirectoryRelation::VMCameraDirectoryRelation(std::string relationId,
                                                     std::string parentId,
                                                     std::string childId,
                                                     RelationType type)
    : mRelationId(std::move(relationId))
    , mParentId(std::move(parentId))
    , mChildId(std::move(childId))
    , mRelationType(type)
{
}

std::string                                       VMCameraDirectoryRelation::getRelationId()   const { return mRelationId; }
std::string                                       VMCameraDirectoryRelation::getParentId()     const { return mParentId; }
std::string                                       VMCameraDirectoryRelation::getChildId()      const { return mChildId; }
VMCameraDirectoryRelation::RelationType           VMCameraDirectoryRelation::getRelationType() const { return mRelationType; }

} // namespace commonHead::viewModels::utils
