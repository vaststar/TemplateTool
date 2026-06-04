#include "CameraDirectoryTreeModel.h"

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
    // id / type are identity; only display fields refresh.
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
    const auto& childId = child->getNodeData().id;
    if (!childId.empty() && m_childIndex.count(childId))
    {
        return; // already a child
    }
    child->setParent(shared_from_this());
    m_childIndex.emplace(childId, m_children.size());
    m_children.push_back(child);
}

void CameraDirectoryTreeNode::removeChild(const std::string& childId)
{
    auto it = m_childIndex.find(childId);
    if (it == m_childIndex.end())
    {
        return;
    }
    const std::size_t idx = it->second;
    m_childIndex.erase(it);

    if (m_children[idx])
    {
        m_children[idx]->clearParent();
    }
    // Swap-remove: O(1) erase from vector, then fix the moved entry's index.
    const std::size_t last = m_children.size() - 1;
    if (idx != last)
    {
        m_children[idx] = std::move(m_children[last]);
        if (m_children[idx])
        {
            m_childIndex[m_children[idx]->getNodeData().id] = idx;
        }
    }
    m_children.pop_back();
}

std::vector<std::shared_ptr<CameraDirectoryTreeNode>> CameraDirectoryTreeNode::takeChildren()
{
    std::vector<std::shared_ptr<CameraDirectoryTreeNode>> out;
    out.swap(m_children);
    m_childIndex.clear();
    for (auto& c : out)
    {
        if (c) c->clearParent();
    }
    return out;
}

std::string CameraDirectoryTreeNode::getRelationId() const
{
    return m_relationId;
}

void CameraDirectoryTreeNode::setRelationId(std::string relationId)
{
    m_relationId = std::move(relationId);
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
}

void CameraDirectoryTree::buildGroupNodes(const ucf::service::model::CameraGroupArray& groups)
{
    for (const auto& group : groups)
    {
        if (!group) continue;
        const std::string id = group->getNodeId();
        if (id.empty()) continue;
        CameraDirectoryNodeData data;
        data.id          = id;
        data.displayName = group->getDisplayName();
        data.type        = CameraDirectoryNodeType::Group;
        data.status      = static_cast<CameraNodeStatus>(static_cast<int>(group->getNodeStatus()));
        m_nodes.emplace(id, std::make_shared<CameraDirectoryTreeNode>(data));
    }
}

void CameraDirectoryTree::buildCameraNodes(const ucf::service::model::CameraEntryArray& cameras)
{
    for (const auto& camera : cameras)
    {
        if (!camera) continue;
        const std::string id = camera->getNodeId();
        if (id.empty()) continue;
        CameraDirectoryNodeData data;
        data.id          = id;
        data.displayName = camera->getDisplayName();
        data.type        = CameraDirectoryNodeType::Camera;
        data.status      = static_cast<CameraNodeStatus>(static_cast<int>(camera->getNodeStatus()));
        m_nodes.emplace(id, std::make_shared<CameraDirectoryTreeNode>(data));
    }
}

void CameraDirectoryTree::buildRelations(const ucf::service::model::CameraDirectoryRelationArray& relations)
{
    for (const auto& rel : relations)
    {
        if (!rel) continue;
        const std::string parentId = rel->getParentId();
        if (parentId.empty()) continue;
        auto itParent = m_nodes.find(parentId);
        if (itParent == m_nodes.end() || !itParent->second) continue;

        const std::string childId = rel->getChildId();
        auto itChild = m_nodes.find(childId);
        if (itChild == m_nodes.end() || !itChild->second) continue;
        itParent->second->addChild(itChild->second);
        itChild->second->setRelationId(rel->getRelationId());
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
        if (!node) continue;
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
    auto it = m_nodes.find(id);
    if (it != m_nodes.end() && it->second)
    {
        return std::static_pointer_cast<ICameraDirectoryTreeNode>(it->second);
    }
    return nullptr;
}

// ===== Helpers =====

void CameraDirectoryTree::detachFromParent(const std::shared_ptr<CameraDirectoryTreeNode>& node)
{
    if (!node) return;
    if (auto parent = std::dynamic_pointer_cast<CameraDirectoryTreeNode>(node->getParent().lock()))
    {
        parent->removeChild(node->getNodeData().id);
    }
    else if (m_root)
    {
        m_root->removeChild(node->getNodeData().id);
    }
}

// ===== Incremental mutations (single) =====

void CameraDirectoryTree::addNode(const CameraDirectoryNodeData& data)
{
    if (data.id.empty() || m_nodes.count(data.id)) return;
    auto node = std::make_shared<CameraDirectoryTreeNode>(data);
    m_nodes.emplace(data.id, node);
    m_root->addChild(node);
}

void CameraDirectoryTree::updateNode(const CameraDirectoryNodeData& data)
{
    auto it = m_nodes.find(data.id);
    if (it == m_nodes.end() || !it->second) return;
    it->second->setNodeData(data);
}

void CameraDirectoryTree::removeNode(const std::string& nodeId)
{
    auto it = m_nodes.find(nodeId);
    if (it == m_nodes.end() || !it->second)
    {
        if (it != m_nodes.end()) m_nodes.erase(it);
        return;
    }
    auto node = it->second;

    // Re-parent orphaned children to virtual root before deleting this node.
    auto orphans = node->takeChildren();
    for (auto& child : orphans)
    {
        if (child) m_root->addChild(child);
    }

    detachFromParent(node);
    m_nodes.erase(it);
}

void CameraDirectoryTree::setRelation(const CameraDirectoryRelationData& relation)
{
    auto itChild = m_nodes.find(relation.childId);
    if (itChild == m_nodes.end() || !itChild->second) return;
    auto child = itChild->second;

    detachFromParent(child);

    auto itParent = m_nodes.find(relation.parentId);
    if (itParent != m_nodes.end() && itParent->second && itParent->second != child)
    {
        itParent->second->addChild(child);
        child->setRelationId(relation.id);
    }
    else
    {
        // Parent unknown / invalid: park on virtual root (no relation row).
        m_root->addChild(child);
        child->setRelationId(std::string{});
    }
}

void CameraDirectoryTree::clearRelation(const std::string& childId)
{
    auto it = m_nodes.find(childId);
    if (it == m_nodes.end() || !it->second) return;
    auto child = it->second;
    detachFromParent(child);
    m_root->addChild(child);
    child->setRelationId(std::string{});
}

// ===== Batch variants =====

void CameraDirectoryTree::addNodes(const std::vector<CameraDirectoryNodeData>& datas)
{
    m_nodes.reserve(m_nodes.size() + datas.size());
    for (const auto& d : datas) addNode(d);
}

void CameraDirectoryTree::updateNodes(const std::vector<CameraDirectoryNodeData>& datas)
{
    for (const auto& d : datas) updateNode(d);
}

void CameraDirectoryTree::removeNodes(const std::vector<std::string>& nodeIds)
{
    for (const auto& id : nodeIds) removeNode(id);
}

void CameraDirectoryTree::setRelations(const std::vector<CameraDirectoryRelationData>& relations)
{
    for (const auto& r : relations)
    {
        setRelation(r);
    }
}

void CameraDirectoryTree::clearRelations(const std::vector<std::string>& childIds)
{
    for (const auto& id : childIds) clearRelation(id);
}

std::string CameraDirectoryTree::getRelationIdByChildId(const std::string& childId) const
{
    auto it = m_nodes.find(childId);
    if (it == m_nodes.end() || !it->second) return {};
    return it->second->getRelationId();
}

std::string CameraDirectoryTree::takeChildIdByRelationId(const std::string& relationId)
{
    if (relationId.empty()) return {};
    // Linear scan is acceptable here: relation removals are rare and only the small set
    // of nodes currently parented in this VM's slice is examined. Adding a reverse index
    // would double bookkeeping cost on every setRelation for no real win.
    for (auto& [childId, node] : m_nodes)
    {
        if (node && node->getRelationId() == relationId)
        {
            node->setRelationId(std::string{});
            return childId;
        }
    }
    return {};
}

}
