#include "SettingsModel.h"

namespace commonHead::viewModels::model {

// ============== SettingsTreeNode ==============

SettingsTreeNode::SettingsTreeNode(const SettingsNodeData& data)
    : m_data(data)
{
}

SettingsNodeData SettingsTreeNode::getNodeData() const
{
    return m_data;
}

std::weak_ptr<ISettingsTreeNode> SettingsTreeNode::getParent() const
{
    if (auto p = m_parent.lock())
    {
        return std::static_pointer_cast<ISettingsTreeNode>(p);
    }
    return std::weak_ptr<ISettingsTreeNode>();
}

std::size_t SettingsTreeNode::getChildCount() const
{
    return m_children.size();
}

std::shared_ptr<ISettingsTreeNode> SettingsTreeNode::getChild(std::size_t index) const
{
    if (index < m_children.size())
    {
        return std::static_pointer_cast<ISettingsTreeNode>(m_children[index]);
    }
    return nullptr;
}

void SettingsTreeNode::setParent(const std::shared_ptr<SettingsTreeNode>& parent)
{
    m_parent = parent;
}

void SettingsTreeNode::addChild(const std::shared_ptr<SettingsTreeNode>& child)
{
    if (!child)
    {
        return;
    }
    child->setParent(shared_from_this());
    m_children.push_back(child);
}

// ============== SettingsTree ==============

void SettingsTree::createVirtualRoot()
{
    SettingsNodeData rootData;
    rootData.nodeId = "";
    rootData.title = "";
    rootData.panelType = SettingsPanelType::None;
    m_root = std::make_shared<SettingsTreeNode>(rootData);
}

SettingsTreeNodePtr SettingsTree::getRoot() const
{
    return std::static_pointer_cast<ISettingsTreeNode>(m_root);
}

SettingsTreeNodePtr SettingsTree::findNodeById(const std::string& nodeId) const
{
    auto it = m_index.find(nodeId);
    if (it != m_index.end())
    {
        if (auto ptr = it->second.lock())
        {
            return std::static_pointer_cast<ISettingsTreeNode>(ptr);
        }
    }
    return nullptr;
}

std::shared_ptr<SettingsTreeNode> SettingsTree::addNode(
    const std::string& parentId, const SettingsNodeData& data)
{
    // Create virtual root on first add
    if (!m_root)
    {
        createVirtualRoot();
    }

    std::shared_ptr<SettingsTreeNode> parent;

    if (parentId.empty())
    {
        // Add to virtual root
        parent = m_root;
    }
    else
    {
        auto it = m_index.find(parentId);
        if (it == m_index.end())
        {
            return nullptr;
        }
        parent = it->second.lock();
        if (!parent)
        {
            return nullptr;
        }
    }

    auto node = std::make_shared<SettingsTreeNode>(data);
    parent->addChild(node);
    m_index[data.nodeId] = node;
    return node;
}

} // namespace commonHead::viewModels::model
