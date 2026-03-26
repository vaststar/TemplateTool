#include "ToolsModel.h"

#include <algorithm>

namespace commonHead::viewModels::model {

// ============== ToolsTreeNode ==============

ToolsTreeNode::ToolsTreeNode(const ToolNodeData& data)
    : m_data(data)
{
}

ToolNodeData ToolsTreeNode::getNodeData() const
{
    return m_data;
}

void ToolsTreeNode::setNodeData(const ToolNodeData& data)
{
    m_data = data;
}

std::weak_ptr<IToolsTreeNode> ToolsTreeNode::getParent() const
{
    if (auto p = m_parent.lock())
    {
        return std::static_pointer_cast<IToolsTreeNode>(p);
    }
    return std::weak_ptr<IToolsTreeNode>();
}

std::size_t ToolsTreeNode::getChildCount() const
{
    return m_children.size();
}

std::shared_ptr<IToolsTreeNode> ToolsTreeNode::getChild(std::size_t index) const
{
    if (index < m_children.size())
    {
        return std::static_pointer_cast<IToolsTreeNode>(m_children[index]);
    }
    return nullptr;
}

void ToolsTreeNode::setParent(const std::shared_ptr<ToolsTreeNode>& parent)
{
    m_parent = parent;
}

void ToolsTreeNode::addChild(const std::shared_ptr<ToolsTreeNode>& child)
{
    if (!child)
    {
        return;
    }
    child->setParent(shared_from_this());
    m_children.push_back(child);
}

bool ToolsTreeNode::removeChild(const std::shared_ptr<ToolsTreeNode>& child)
{
    if (!child)
    {
        return false;
    }
    auto it = std::find(m_children.begin(), m_children.end(), child);
    if (it == m_children.end())
    {
        return false;
    }
    (*it)->setParent(nullptr);
    m_children.erase(it);
    return true;
}

// ============== ToolsTree ==============

void ToolsTree::createVirtualRoot()
{
    ToolNodeData rootData;
    rootData.nodeId = "";
    rootData.title = "";
    rootData.panelType = ToolPanelType::None;
    m_root = std::make_shared<ToolsTreeNode>(rootData);
}

ToolsTreeNodePtr ToolsTree::getRoot() const
{
    return std::static_pointer_cast<IToolsTreeNode>(m_root);
}

ToolsTreeNodePtr ToolsTree::findNodeById(const std::string& nodeId) const
{
    auto it = m_index.find(nodeId);
    if (it != m_index.end())
    {
        if (auto ptr = it->second.lock())
        {
            return std::static_pointer_cast<IToolsTreeNode>(ptr);
        }
    }
    return nullptr;
}

std::shared_ptr<ToolsTreeNode> ToolsTree::addNode(
    const std::string& parentId, const ToolNodeData& data)
{
    // Create virtual root on first add
    if (!m_root)
    {
        createVirtualRoot();
    }

    std::shared_ptr<ToolsTreeNode> parent;

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

    auto node = std::make_shared<ToolsTreeNode>(data);
    parent->addChild(node);
    m_index[data.nodeId] = node;
    return node;
}

bool ToolsTree::removeNode(const std::string& nodeId)
{
    auto it = m_index.find(nodeId);
    if (it == m_index.end())
    {
        return false;
    }

    auto node = it->second.lock();
    if (!node)
    {
        m_index.erase(it);
        return false;
    }

    auto parent = node->getParent().lock();
    if (!parent)
    {
        return false;
    }

    // Downcast parent to concrete type to call removeChild
    auto parentConcrete = std::dynamic_pointer_cast<ToolsTreeNode>(parent);
    if (!parentConcrete)
    {
        return false;
    }

    if (!parentConcrete->removeChild(node))
    {
        return false;
    }

    m_index.erase(it);
    return true;
}

} // namespace commonHead::viewModels::model
