#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <commonHead/viewModels/SettingsViewModel/ISettingsModel.h>

namespace commonHead::viewModels::model {

class SettingsTreeNode : public ISettingsTreeNode,
                         public std::enable_shared_from_this<SettingsTreeNode>
{
public:
    explicit SettingsTreeNode(const SettingsNodeData& data);

    SettingsNodeData getNodeData() const override;
    void setNodeData(const SettingsNodeData& data) override;
    std::weak_ptr<ISettingsTreeNode> getParent() const override;
    std::size_t getChildCount() const override;
    std::shared_ptr<ISettingsTreeNode> getChild(std::size_t index) const override;

    void setParent(const std::shared_ptr<SettingsTreeNode>& parent);
    void addChild(const std::shared_ptr<SettingsTreeNode>& child);
    bool removeChild(const std::shared_ptr<SettingsTreeNode>& child);

private:
    SettingsNodeData m_data;
    std::weak_ptr<SettingsTreeNode> m_parent;
    std::vector<std::shared_ptr<SettingsTreeNode>> m_children;
};

class SettingsTree : public ISettingsTree
{
public:
    SettingsTree() = default;

    SettingsTreeNodePtr getRoot() const override;
    SettingsTreeNodePtr findNodeById(const std::string& nodeId) const override;
    bool removeNode(const std::string& nodeId) override;

    // Builder - add node under parent (empty parentId means add to virtual root)
    std::shared_ptr<SettingsTreeNode> addNode(
        const std::string& parentId, const SettingsNodeData& data);

private:
    void createVirtualRoot();

private:
    std::shared_ptr<SettingsTreeNode> m_root;
    std::unordered_map<std::string, std::weak_ptr<SettingsTreeNode>> m_index;
};

} // namespace commonHead::viewModels::model
