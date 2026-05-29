#include "CameraDirectoryTreeModel.h"

#include <algorithm>

namespace commonHead::viewModels::model {

CameraDirectoryTreeNode::CameraDirectoryTreeNode(const CameraDirectoryNodeData& data)
    : m_data(data)
{
}

CameraDirectoryNodeData CameraDirectoryTreeNode::getNodeData() const
{
    return m_data;
}

std::weak_ptr<ICameraDirectoryTreeNode> CameraDirectoryTreeNode::getParent() const
{
    if (auto p = m_parent.lock())
    {
        return std::static_pointer_cast<ICameraDirectoryTreeNode>(p);
    }
    return std::weak_ptr<ICameraDirectoryTreeNode>();
}

std::size_t CameraDirectoryTreeNode::getChildCount() const
{
    return m_children.size();
}

std::shared_ptr<ICameraDirectoryTreeNode> CameraDirectoryTreeNode::getChild(std::size_t index) const
{
    if (index < m_children.size())
    {
        return std::static_pointer_cast<ICameraDirectoryTreeNode>(m_children[index]);
    }
    return nullptr;
}

void CameraDirectoryTreeNode::setNodeData(const CameraDirectoryNodeData& data)
{
    // id / type 是节点身份，禁止变更；仅刷新展示字段
    m_data.displayName = data.displayName;
    m_data.status      = data.status;
}

void CameraDirectoryTreeNode::setParent(const std::shared_ptr<CameraDirectoryTreeNode>& parent)
{
    m_parent = parent;
}

void CameraDirectoryTreeNode::clearParent()
{
    m_parent.reset();
}

void CameraDirectoryTreeNode::addChild(const std::shared_ptr<CameraDirectoryTreeNode>& child)
{
    if (!child)
    {
        return;
    }
    child->setParent(shared_from_this());
    m_children.push_back(child);
}

void CameraDirectoryTreeNode::removeChild(const std::shared_ptr<CameraDirectoryTreeNode>& child)
{
    if (!child)
    {
        return;
    }
    auto it = std::remove(m_children.begin(), m_children.end(), child);
    if (it != m_children.end())
    {
        m_children.erase(it, m_children.end());
        child->clearParent();
    }
}


// ---------------- CameraDirectoryTree ----------------

CameraDirectoryTree::CameraDirectoryTree(const ucf::service::model::CameraGroupArray& groups,
                                         const ucf::service::model::CameraEntryArray& cameras,
                                         const ucf::service::model::CameraDirectoryRelationArray& relations)
{
    buildGroupNodes(groups);
    buildCameraNodes(cameras);
    buildRelations(relations);
    createVirtualRoot();
    rebuildIndex();
}

void CameraDirectoryTree::rebuildIndex()
{
    m_index.clear();
    m_index.reserve(m_nodes.size());
    for (const auto& [id, node] : m_nodes)
    {
        m_index.emplace(id, node);
    }
}

void CameraDirectoryTree::buildGroupNodes(const ucf::service::model::CameraGroupArray& groups)
{
    for (const auto& group : groups)
    {
        if (!group)
        {
            continue;
        }
        const std::string id = group->getNodeId();
        if (id.empty())
        {
            continue;
        }
        CameraDirectoryNodeData data;
        data.id          = id;
        data.displayName = group->getDisplayName();
        data.type        = CameraDirectoryNodeType::Group;

        m_nodes.emplace(id, std::make_shared<CameraDirectoryTreeNode>(data));
    }
}

void CameraDirectoryTree::buildCameraNodes(const ucf::service::model::CameraEntryArray& cameras)
{
    for (const auto& camera : cameras)
    {
        if (!camera)
        {
            continue;
        }
        const std::string id = camera->getNodeId();
        if (id.empty())
        {
            continue;
        }
        CameraDirectoryNodeData data;
        data.id          = id;
        data.displayName = camera->getDisplayName();
        data.type        = CameraDirectoryNodeType::Camera;

        m_nodes.emplace(id, std::make_shared<CameraDirectoryTreeNode>(data));
    }
}

void CameraDirectoryTree::buildRelations(const ucf::service::model::CameraDirectoryRelationArray& relations)
{
    for (const auto& rel : relations)
    {
        if (!rel)
        {
            continue;
        }
        const std::string parentId = rel->getParentId();
        if (parentId.empty())
        {
            continue;
        }
        auto itParent = m_nodes.find(parentId);
        if (itParent == m_nodes.end())
        {
            continue;
        }
        auto parentNode = itParent->second;
        if (!parentNode)
        {
            continue;
        }

        const std::string childId = rel->getChildId();
        auto itChild = m_nodes.find(childId);
        if (itChild == m_nodes.end())
        {
            continue;
        }
        auto childNode = itChild->second;
        if (!childNode)
        {
            continue;
        }
        parentNode->addChild(childNode);
    }
}

void CameraDirectoryTree::createVirtualRoot()
{
    CameraDirectoryNodeData rootData;
    rootData.id          = "";
    rootData.displayName = "";
    rootData.type        = CameraDirectoryNodeType::Group;

    m_root = std::make_shared<CameraDirectoryTreeNode>(rootData);

    for (const auto& [_, node] : m_nodes)
    {
        if (!node)
        {
            continue;
        }
        if (auto parent = node->getParent().lock(); !parent)
        {
            m_root->addChild(node);
        }
    }
}

CameraDirectoryTreeNodePtr CameraDirectoryTree::getRoot() const
{
    return std::static_pointer_cast<ICameraDirectoryTreeNode>(m_root);
}

CameraDirectoryTreeNodePtr CameraDirectoryTree::findNodeById(const std::string& id) const
{
    if (auto it = m_index.find(id); it != m_index.end())
    {
        if (auto ptr = it->second.lock())
        {
            return std::static_pointer_cast<ICameraDirectoryTreeNode>(ptr);
        }
    }
    return nullptr;
}

// ===== Incremental mutators =====

void CameraDirectoryTree::attachToVirtualRoot(const std::shared_ptr<CameraDirectoryTreeNode>& node)
{
    if (!node || !m_root)
    {
        return;
    }
    m_root->addChild(node);
}

void CameraDirectoryTree::detachFromCurrentParent(const std::shared_ptr<CameraDirectoryTreeNode>& node)
{
    if (!node)
    {
        return;
    }
    if (auto parent = node->getParent().lock())
    {
        // parent 是 ICameraDirectoryTreeNode；这里 cast 回 impl 类型以调用 removeChild
        auto parentImpl = std::static_pointer_cast<CameraDirectoryTreeNode>(parent);
        parentImpl->removeChild(node);
    }
}

void CameraDirectoryTree::addNode(const CameraDirectoryNodeData& data)
{
    if (data.id.empty() || m_nodes.find(data.id) != m_nodes.end())
    {
        return;
    }
    auto node = std::make_shared<CameraDirectoryTreeNode>(data);
    m_nodes.emplace(data.id, node);
    m_index.emplace(data.id, node);
    attachToVirtualRoot(node);
}

void CameraDirectoryTree::updateNode(const CameraDirectoryNodeData& data)
{
    auto it = m_nodes.find(data.id);
    if (it == m_nodes.end() || !it->second)
    {
        return;
    }
    it->second->setNodeData(data);
}

void CameraDirectoryTree::removeNode(const std::string& nodeId)
{
    auto it = m_nodes.find(nodeId);
    if (it == m_nodes.end())
    {
        return;
    }
    auto node = it->second;
    if (node)
    {
        detachFromCurrentParent(node);
        // node 的 children 此时仍指向 m_children 中的 shared_ptr，且这些子节点的 parent 指针仍指向被删 node。
        // 这里采取保守策略：把 children 全部挂回 virtual root，由后续 relation 事件再重组。
        for (std::size_t i = 0; i < node->getChildCount(); ++i)
        {
            auto child = std::static_pointer_cast<CameraDirectoryTreeNode>(node->getChild(i));
            if (child)
            {
                child->clearParent();
                attachToVirtualRoot(child);
            }
        }
    }
    m_nodes.erase(it);
    m_index.erase(nodeId);
}

void CameraDirectoryTree::setRelation(const std::string& parentId, const std::string& childId)
{
    auto itChild = m_nodes.find(childId);
    if (itChild == m_nodes.end() || !itChild->second)
    {
        return;
    }
    auto child = itChild->second;

    detachFromCurrentParent(child);

    auto itParent = m_nodes.find(parentId);
    if (itParent != m_nodes.end() && itParent->second)
    {
        itParent->second->addChild(child);
    }
    else
    {
        // 父节点不存在：暂挂 virtual root，等父节点出现时由后续事件再 reparent
        attachToVirtualRoot(child);
    }
}

void CameraDirectoryTree::clearRelation(const std::string& childId)
{
    auto it = m_nodes.find(childId);
    if (it == m_nodes.end() || !it->second)
    {
        return;
    }
    auto child = it->second;
    detachFromCurrentParent(child);
    attachToVirtualRoot(child);
}

}
