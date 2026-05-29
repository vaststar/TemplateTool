#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>

namespace commonHead::viewModels::model {

enum class COMMONHEAD_EXPORT CameraDirectoryNodeType
{
    Group,
    Camera
};

enum class COMMONHEAD_EXPORT CameraNodeStatus
{
    Active   = 0,
    Inactive = 1,
    Deleted  = 2,
    Archived = 3
};

struct COMMONHEAD_EXPORT CameraDirectoryNodeData
{
    std::string id;
    std::string displayName;
    CameraDirectoryNodeType type = CameraDirectoryNodeType::Group;
    CameraNodeStatus status      = CameraNodeStatus::Active;
};

// VM 层"关系"载荷：父子节点 id 对。
// service 端 ICameraDirectoryRelation 的 VM 投影，作为增量回调的 payload。
struct COMMONHEAD_EXPORT CameraDirectoryRelationData
{
    std::string parentId;
    std::string childId;
};

class COMMONHEAD_EXPORT ICameraDirectoryTreeNode
{
public:
    virtual ~ICameraDirectoryTreeNode() = default;

    virtual CameraDirectoryNodeData getNodeData() const = 0;
    virtual std::weak_ptr<ICameraDirectoryTreeNode> getParent() const = 0;
    virtual std::size_t getChildCount() const = 0;
    virtual std::shared_ptr<ICameraDirectoryTreeNode> getChild(std::size_t index) const = 0;
};

using CameraDirectoryTreeNodePtr = std::shared_ptr<ICameraDirectoryTreeNode>;

class COMMONHEAD_EXPORT ICameraDirectoryTree
{
public:
    virtual ~ICameraDirectoryTree() = default;

    virtual CameraDirectoryTreeNodePtr getRoot() const = 0;
    virtual CameraDirectoryTreeNodePtr findNodeById(const std::string& id) const = 0;
};

using CameraDirectoryTreePtr = std::shared_ptr<ICameraDirectoryTree>;

}
