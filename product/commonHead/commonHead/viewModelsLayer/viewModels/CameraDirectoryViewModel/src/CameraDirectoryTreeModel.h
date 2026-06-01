#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <ucf/Services/CameraDirectoryService/ICameraDirectoryEntities.h>

#include <commonHead/viewModels/CameraDirectoryViewModel/ICameraDirectoryTreeModel.h>

namespace commonHead::viewModels::model {

class CameraDirectoryTreeNode
    : public ICameraDirectoryTreeNode
    , public std::enable_shared_from_this<CameraDirectoryTreeNode>
{
public:
    explicit CameraDirectoryTreeNode(const CameraDirectoryNodeData& data);

    CameraDirectoryNodeData getNodeData() const override;
    std::weak_ptr<ICameraDirectoryTreeNode> getParent() const override;
    std::size_t getChildCount() const override;
    std::shared_ptr<ICameraDirectoryTreeNode> getChild(std::size_t index) const override;

    void setNodeData(const CameraDirectoryNodeData& data);
    void setParent(const std::shared_ptr<CameraDirectoryTreeNode>& parent);
    void clearParent();
    void addChild(const std::shared_ptr<CameraDirectoryTreeNode>& child);
    void removeChild(const std::string& childId);
    // Move all children out (used when re-parenting orphans to virtual root)
    std::vector<std::shared_ptr<CameraDirectoryTreeNode>> takeChildren();

private:
    CameraDirectoryNodeData m_data;
    std::weak_ptr<CameraDirectoryTreeNode> m_parent;
    std::vector<std::shared_ptr<CameraDirectoryTreeNode>> m_children;
    std::unordered_map<std::string, std::size_t> m_childIndex; // id -> position in m_children
};

class CameraDirectoryTree : public ICameraDirectoryTree
{
public:
    CameraDirectoryTree(const ucf::service::model::CameraGroupArray& groups,
                        const ucf::service::model::CameraEntryArray& cameras,
                        const ucf::service::model::CameraDirectoryRelationArray& relations);

    CameraDirectoryTreeNodePtr getRoot() const override;
    CameraDirectoryTreeNodePtr findNodeById(const std::string& id) const override;

    // Incremental mutations (single)
    void addNode(const CameraDirectoryNodeData& data);
    void updateNode(const CameraDirectoryNodeData& data);
    void removeNode(const std::string& nodeId);
    void setRelation(const std::string& parentId, const std::string& childId);
    void clearRelation(const std::string& childId);

    // Batch variants (preferred for callback payloads)
    void addNodes(const std::vector<CameraDirectoryNodeData>& datas);
    void updateNodes(const std::vector<CameraDirectoryNodeData>& datas);
    void removeNodes(const std::vector<std::string>& nodeIds);
    void setRelations(const std::vector<std::pair<std::string, std::string>>& parentChildPairs);
    void clearRelations(const std::vector<std::string>& childIds);

private:
    void buildGroupNodes(const ucf::service::model::CameraGroupArray& groups);
    void buildCameraNodes(const ucf::service::model::CameraEntryArray& cameras);
    void buildRelations(const ucf::service::model::CameraDirectoryRelationArray& relations);
    void createVirtualRoot();
    void detachFromParent(const std::shared_ptr<CameraDirectoryTreeNode>& node);

private:
    std::shared_ptr<CameraDirectoryTreeNode> m_root;
    std::unordered_map<std::string, std::shared_ptr<CameraDirectoryTreeNode>> m_nodes;
};

}
