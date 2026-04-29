#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <commonHead/viewModels/ToolsViewModel/IToolsModel.h>

namespace commonHead::viewModels::model {

class ToolsTreeNode : public IToolsTreeNode,
                      public std::enable_shared_from_this<ToolsTreeNode>
{
public:
    explicit ToolsTreeNode(const ToolNodeData& data);

    ToolNodeData getNodeData() const override;
    void setNodeData(const ToolNodeData& data) override;
    std::weak_ptr<IToolsTreeNode> getParent() const override;
    std::size_t getChildCount() const override;
    std::shared_ptr<IToolsTreeNode> getChild(std::size_t index) const override;

    void setParent(const std::shared_ptr<ToolsTreeNode>& parent);
    void addChild(const std::shared_ptr<ToolsTreeNode>& child);
    bool removeChild(const std::shared_ptr<ToolsTreeNode>& child);

private:
    ToolNodeData m_data;
    std::weak_ptr<ToolsTreeNode> m_parent;
    std::vector<std::shared_ptr<ToolsTreeNode>> m_children;
};

class ToolsTree : public IToolsTree
{
public:
    ToolsTree() = default;

    ToolsTreeNodePtr getRoot() const override;
    ToolsTreeNodePtr findNodeById(const std::string& nodeId) const override;
    bool removeNode(const std::string& nodeId) override;

    // Builder - add node under parent (empty parentId means add to virtual root)
    std::shared_ptr<ToolsTreeNode> addNode(
        const std::string& parentId, const ToolNodeData& data);

private:
    void createVirtualRoot();

private:
    std::shared_ptr<ToolsTreeNode> m_root;
    std::unordered_map<std::string, std::weak_ptr<ToolsTreeNode>> m_index;
};

} // namespace commonHead::viewModels::model
