#pragma once

#include <memory>
#include <string>
#include <vector>

#include <ucf/Services/CameraDirectoryService/ICameraDirectoryEntities.h>
#include <ucf/Services/CameraDirectoryService/ICameraDirectoryServiceCallback.h>
#include <ucf/Services/MediaService/MediaTypes.h>

#include <commonHead/viewModels/CameraDirectoryViewModel/ICameraDirectoryTreeModel.h>
#include <commonHead/viewModels/MediaCameraViewModel/CameraSource.h>

namespace commonHead::viewModels::model {
    class CameraDirectoryTree;
}

namespace commonHead::viewModels::utils {

// ===== Source / status / load-error converters =====
ucf::service::media::CameraSource toServiceSource(const model::CameraSource& vmSource);
model::CameraSource               toVMSource(const ucf::service::media::CameraSource& serviceSource);
model::CameraNodeStatus           toVMStatus(ucf::service::model::CameraNodeStatus serviceStatus);
model::CameraDirectoryLoadError   toVMLoadError(ucf::service::CameraDirectoryLoadError serviceError);

// ===== Service-entity -> VM-data converters =====
model::CameraDirectoryNodeData     toVMNodeData (const ucf::service::model::ICameraGroupPtr& group);
model::CameraDirectoryNodeData     toVMNodeData (const ucf::service::model::ICameraEntryPtr& camera);
model::CameraDirectoryRelationData toVMRelation (const ucf::service::model::ICameraDirectoryRelationPtr& serviceRelation);

std::vector<model::CameraDirectoryNodeData>     toVMNodeDatas(const ucf::service::model::CameraGroupArray& groups);
std::vector<model::CameraDirectoryNodeData>     toVMNodeDatas(const ucf::service::model::CameraEntryArray& cameras);
std::vector<model::CameraDirectoryRelationData> toVMRelations(const ucf::service::model::CameraDirectoryRelationArray& serviceRelations);

// ===== Tree helpers =====
// Walk up from startNodeId; return true if ancestorCandidateId appears among the
// ancestors (excluding startNodeId itself). Used to reject re-parent cycles.
bool isAncestorOf(const std::shared_ptr<model::CameraDirectoryTree>& tree,
                  const std::string& ancestorCandidateId,
                  const std::string& startNodeId);

// ===== Minimal ucf::service::model::* impls owned by the VM =====
// These let the VM synthesize service-shaped DTOs to push down through the service
// write APIs (addCameraGroups / addCameras / addCameraRelations / ...) without
// pulling in the service-private CameraDirectoryEntities.h. The service copies the
// fields into its own internal Impl on persist, so these can stay lock-free.

class VMCameraGroup final : public ucf::service::model::ICameraGroup
{
public:
    VMCameraGroup(std::string nodeId,
                  std::string displayName,
                  ucf::service::model::CameraNodeStatus status = ucf::service::model::CameraNodeStatus::Active);

    std::string                            getNodeId()      const override;
    std::string                            getDisplayName() const override;
    ucf::service::model::CameraNodeStatus  getNodeStatus()  const override;

private:
    std::string                            mNodeId;
    std::string                            mDisplayName;
    ucf::service::model::CameraNodeStatus  mStatus;
};

class VMCameraEntry final : public ucf::service::model::ICameraEntry
{
public:
    VMCameraEntry(std::string nodeId,
                  std::string displayName,
                  ucf::service::media::CameraSource source,
                  ucf::service::model::CameraNodeStatus status = ucf::service::model::CameraNodeStatus::Active);

    std::string                            getNodeId()      const override;
    std::string                            getDisplayName() const override;
    ucf::service::model::CameraNodeStatus  getNodeStatus()  const override;
    ucf::service::media::CameraSource      getSource()      const override;

private:
    std::string                            mNodeId;
    std::string                            mDisplayName;
    ucf::service::model::CameraNodeStatus  mStatus;
    ucf::service::media::CameraSource      mSource;
};

class VMCameraDirectoryRelation final : public ucf::service::model::ICameraDirectoryRelation
{
public:
    // relationId may be empty when issuing an "add" request; the service mints a new
    // UUID in that case. For update requests callers must pass the existing id.
    VMCameraDirectoryRelation(std::string relationId,
                              std::string parentId,
                              std::string childId,
                              RelationType type = RelationType::Containment);

    std::string  getRelationId()   const override;
    std::string  getParentId()     const override;
    std::string  getChildId()      const override;
    RelationType getRelationType() const override;

private:
    std::string  mRelationId;
    std::string  mParentId;
    std::string  mChildId;
    RelationType mRelationType;
};

} // namespace commonHead::viewModels::utils
