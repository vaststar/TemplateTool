#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include <commonHead/viewModels/CameraDirectoryViewModel/CameraDirectoryViewModelTypesExport.h>

namespace commonHead::viewModels::model {

enum class CameraDirectoryNodeType
{
    Group,
    Camera
};

enum class CameraNodeStatus
{
    Active   = 0,
    Inactive = 1,
    Deleted  = 2,
    Archived = 3
};

struct CAMERA_DIRECTORY_VIEW_MODEL_TYPES_API CameraDirectoryNodeData
{
    std::string id;
    std::string displayName;
    CameraDirectoryNodeType type = CameraDirectoryNodeType::Group;
    CameraNodeStatus status      = CameraNodeStatus::Active;
};

// VM-level relation payload: parent/child node id pair plus the surrogate primary key.
// VM-side projection of the service's ICameraDirectoryRelation, used as the payload of
// incremental relation callbacks.
struct CAMERA_DIRECTORY_VIEW_MODEL_TYPES_API CameraDirectoryRelationData
{
    // The relation's surrogate primary key (RELATION_ID, UUID). May be empty when the
    // caller is issuing a brand-new "addRelation" request and asks the service to mint
    // one. After a successful add/update round-trip this field is populated.
    std::string id;
    std::string parentId;
    std::string childId;
};

// VM-level mirror of the service's CameraDirectoryLoadError. Reported via
// ICameraDirectoryViewModelCallback::onCameraDirectoryLoadFailed.
enum class CameraDirectoryLoadError
{
    Unknown,
    DatabaseNotBound,
    DatabaseReadFailed,
};

class CAMERA_DIRECTORY_VIEW_MODEL_TYPES_API ICameraDirectoryTreeNode
{
public:
    virtual ~ICameraDirectoryTreeNode() = default;

    virtual CameraDirectoryNodeData getNodeData() const = 0;
    virtual std::weak_ptr<ICameraDirectoryTreeNode> getParent() const = 0;
    virtual std::size_t getChildCount() const = 0;
    virtual std::shared_ptr<ICameraDirectoryTreeNode> getChild(std::size_t index) const = 0;
};

using CameraDirectoryTreeNodePtr = std::shared_ptr<ICameraDirectoryTreeNode>;

class CAMERA_DIRECTORY_VIEW_MODEL_TYPES_API ICameraDirectoryTree
{
public:
    virtual ~ICameraDirectoryTree() = default;

    virtual CameraDirectoryTreeNodePtr getRoot() const = 0;
    virtual CameraDirectoryTreeNodePtr findNodeById(const std::string& id) const = 0;
};

using CameraDirectoryTreePtr = std::shared_ptr<ICameraDirectoryTree>;

} // namespace commonHead::viewModels::model
