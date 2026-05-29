#pragma once

#include <memory>
#include <string>
#include <unordered_map>
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
    void removeChild(const std::shared_ptr<CameraDirectoryTreeNode>& child);

private:
    CameraDirectoryNodeData m_data;
    std::weak_ptr<CameraDirectoryTreeNode> m_parent;
    std::vector<std::shared_ptr<CameraDirectoryTreeNode>> m_children;
};

class CameraDirectoryTree : public ICameraDirectoryTree
{
public:
    CameraDirectoryTree(const ucf::service::model::CameraGroupArray& groups,
                        const ucf::service::model::CameraEntryArray& cameras,
                        const ucf::service::model::CameraDirectoryRelationArray& relations);

    CameraDirectoryTreeNodePtr getRoot() const override;
    CameraDirectoryTreeNodePtr findNodeById(const std::string& id) const override;

    // ===== 增量 mutator =====
    // 新增节点：作为孤儿挂到 virtual root，等待后续 relation 事件再 reparent
    void addNode(const CameraDirectoryNodeData& data);
    // 更新节点的 displayName / status 等（节点身份保留）
    void updateNode(const CameraDirectoryNodeData& data);
    // 移除节点：从父挂载点摘下、从索引/容器删除
    void removeNode(const std::string& nodeId);
    // 设置 / 改变父子关系：先从旧父摘下，再挂到新父；新父不存在时挂回 virtual root
    void setRelation(const std::string& parentId, const std::string& childId);
    // 解除关系：把子节点重新挂回 virtual root
    void clearRelation(const std::string& childId);

private:
    void buildGroupNodes(const ucf::service::model::CameraGroupArray& groups);
    void buildCameraNodes(const ucf::service::model::CameraEntryArray& cameras);
    void buildRelations(const ucf::service::model::CameraDirectoryRelationArray& relations);
    void createVirtualRoot();
    void rebuildIndex();
    void attachToVirtualRoot(const std::shared_ptr<CameraDirectoryTreeNode>& node);
    void detachFromCurrentParent(const std::shared_ptr<CameraDirectoryTreeNode>& node);

private:
    std::shared_ptr<CameraDirectoryTreeNode> m_root;
    std::unordered_map<std::string, std::weak_ptr<CameraDirectoryTreeNode>> m_index;
    std::unordered_map<std::string, std::shared_ptr<CameraDirectoryTreeNode>> m_nodes;
};

}
