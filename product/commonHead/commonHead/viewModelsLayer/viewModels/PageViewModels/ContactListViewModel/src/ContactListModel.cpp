
#include "ContactListModel.h"

#include <algorithm>

#include <ucf/Services/ContactService/IContactEntities.h>

namespace commonHead::viewModels::model{

ContactTreeNode::ContactTreeNode(const ContactNodeData& data)
    : m_data(data)
{
}

ContactNodeData ContactTreeNode::getNodeData() const
{
    return m_data;
}

std::weak_ptr<IContactTreeNode> ContactTreeNode::getParent() const
{
    if (auto p = m_parent.lock())
    {
        return std::static_pointer_cast<IContactTreeNode>(p);
    }
    return std::weak_ptr<IContactTreeNode>();
}

std::size_t ContactTreeNode::getChildCount() const
{
    return m_children.size();
}

std::shared_ptr<IContactTreeNode> ContactTreeNode::getChild(std::size_t index) const
{
    if (index < m_children.size())
    {
        return std::static_pointer_cast<IContactTreeNode>(m_children[index]);
    }
    return nullptr;
}

void ContactTreeNode::setParent(const std::shared_ptr<ContactTreeNode>& parent)
{
    m_parent = parent;
}

void ContactTreeNode::addChild(const std::shared_ptr<ContactTreeNode>& child)
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


void ContactTreeNode::removeChild(const std::string& childId)
{
    auto it = m_childIndex.find(childId);
    if (it == m_childIndex.end())
    {
        return;
    }
    const std::size_t idx = it->second;
    m_childIndex.erase(it);

    // Swap-remove: O(1) erase from vector, then fix the moved entry's index
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

void ContactTreeNode::updateData(const ContactNodeData& data)
{
    m_data = data;
}

std::vector<std::shared_ptr<ContactTreeNode>> ContactTreeNode::takeChildren()
{
    std::vector<std::shared_ptr<ContactTreeNode>> out;
    out.swap(m_children);
    m_childIndex.clear();
    return out;
}

const std::string& ContactTreeNode::getRelationId() const
{
    return m_relationId;
}

void ContactTreeNode::setRelationId(std::string relationId)
{
    m_relationId = std::move(relationId);
}


// ---------------- ContactTree ----------------

ContactTree::ContactTree(const std::vector<ucf::service::model::IContactPtr>& contacts, const std::vector<ucf::service::model::IContactRelationPtr>& relations)
{
    buildNodes(contacts);
    buildRelations(relations);
    createVirtualRoot();
}

void ContactTree::createVirtualRoot()
{
    ContactNodeData rootData;
    rootData.id = "";
    rootData.displayName = "";
    rootData.type = ContactNodeType::Group;

    m_root = std::make_shared<ContactTreeNode>(rootData);

    for (const auto& [_, node] : m_nodes)
    {
        if (node)
        {
            if (auto parent = node->getParent().lock(); !parent)
            {
                m_root->addChild(node);
            }
        }
    }
}

void ContactTree::buildNodes(const std::vector<ucf::service::model::IContactPtr>& contacts)
{
    m_nodes.clear();
    m_nodes.reserve(contacts.size());

    for (const auto& contact_data : contacts)
    {
        if (!contact_data)
        {
            continue;
        }
        const std::string id = contact_data->getContactId();
        if (id.empty())
        {
            continue;
        }

        ContactNodeData data;
        data.id = id;

        if (auto person = std::dynamic_pointer_cast<ucf::service::model::IPersonContact>(contact_data))
        {
            data.type = ContactNodeType::Person;
            data.displayName = person->getPersonName();
        }
        else if (auto group = std::dynamic_pointer_cast<ucf::service::model::IGroupContact>(contact_data))
        {
            data.type = ContactNodeType::Group;
            data.displayName = group->getGroupName();
        }
        else
        {
            data.type = ContactNodeType::Person;
            data.displayName = id;
        }

        auto node = std::make_shared<ContactTreeNode>(data);
        m_nodes.emplace(id, node);
    }
}

void ContactTree::buildRelations(const std::vector<ucf::service::model::IContactRelationPtr>& relations)
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
        if (itParent == m_nodes.end() || !itParent->second)
        {
            continue;
        }

        const std::string childId  = rel->getChildId();
        auto itChild = m_nodes.find(childId);
        if (itChild == m_nodes.end() || !itChild->second)
        {
            continue;
        }
        itParent->second->addChild(itChild->second);
        itChild->second->setRelationId(rel->getRelationId());
    }
}

ContactTreeNodePtr ContactTree::getRoot() const
{
    return std::static_pointer_cast<IContactTreeNode>(m_root);
}

ContactTreeNodePtr ContactTree::findNodeById(const std::string& id) const
{
    auto it = m_nodes.find(id);
    if (it != m_nodes.end() && it->second)
    {
        return std::static_pointer_cast<IContactTreeNode>(it->second);
    }
    return nullptr;
}

// ===== Helpers =====

void ContactTree::detachFromParent(const std::shared_ptr<ContactTreeNode>& node)
{
    if (!node)
    {
        return;
    }
    if (auto parent = std::dynamic_pointer_cast<ContactTreeNode>(node->getParent().lock()))
    {
        parent->removeChild(node->getNodeData().id);
    }
    else if (m_root)
    {
        m_root->removeChild(node->getNodeData().id);
    }
}

// ===== Incremental mutations (single) =====

void ContactTree::addNode(const ContactNodeData& data)
{
    if (data.id.empty() || m_nodes.count(data.id))
    {
        return;
    }
    auto node = std::make_shared<ContactTreeNode>(data);
    m_nodes.emplace(data.id, node);
    m_root->addChild(node);
}

void ContactTree::updateNode(const ContactNodeData& data)
{
    auto it = m_nodes.find(data.id);
    if (it == m_nodes.end() || !it->second)
    {
        return;
    }
    it->second->updateData(data);
}

void ContactTree::removeNode(const std::string& id)
{
    auto it = m_nodes.find(id);
    if (it == m_nodes.end() || !it->second)
    {
        if (it != m_nodes.end())
        {
            m_nodes.erase(it);
        }
        return;
    }
    auto node = it->second;

    // Re-parent orphaned children to virtual root before deleting this node
    auto orphans = node->takeChildren();
    for (auto& child : orphans)
    {
        if (child)
        {
            child->setRelationId({});
            m_root->addChild(child);
        }
    }

    detachFromParent(node);
    m_nodes.erase(it);
}

void ContactTree::setRelation(const ContactRelationData& relation)
{
    auto itChild  = m_nodes.find(relation.childId);
    auto itParent = m_nodes.find(relation.parentId);
    if (itChild == m_nodes.end() || itParent == m_nodes.end())
    {
        return;
    }
    auto childNode  = itChild->second;
    auto parentNode = itParent->second;
    if (!childNode || !parentNode || childNode == parentNode)
    {
        return;
    }
    detachFromParent(childNode);
    parentNode->addChild(childNode);
    childNode->setRelationId(relation.id);
}

// ===== Batch variants =====

void ContactTree::addNodes(const std::vector<ContactNodeData>& datas)
{
    m_nodes.reserve(m_nodes.size() + datas.size());
    for (const auto& d : datas)
    {
        addNode(d);
    }
}

void ContactTree::updateNodes(const std::vector<ContactNodeData>& datas)
{
    for (const auto& d : datas)
    {
        updateNode(d);
    }
}

void ContactTree::removeNodes(const std::vector<std::string>& ids)
{
    for (const auto& id : ids)
    {
        removeNode(id);
    }
}

void ContactTree::setRelations(const std::vector<ContactRelationData>& relations)
{
    for (const auto& r : relations)
    {
        setRelation(r);
    }
}

// ===== Relation row identity lookups =====

std::string ContactTree::getRelationIdByChildId(const std::string& childId) const
{
    auto it = m_nodes.find(childId);
    if (it == m_nodes.end() || !it->second)
    {
        return {};
    }
    return it->second->getRelationId();
}

std::vector<RemovedRelationInfo>
ContactTree::removeRelationsByIds(const std::vector<std::string>& relationIds)
{
    std::vector<RemovedRelationInfo> removed;
    removed.reserve(relationIds.size());
    for (const auto& rid : relationIds)
    {
        if (rid.empty())
        {
            continue;
        }
        // O(N) scan over m_nodes; relationIds live only on tree nodes today and the
        // slice size is small. Add a reverse map later if a profile shows it matters.
        std::shared_ptr<ContactTreeNode> childNode;
        std::string                       childId;
        for (const auto& [cid, node] : m_nodes)
        {
            if (node && node->getRelationId() == rid)
            {
                childId   = cid;
                childNode = node;
                break;
            }
        }
        if (!childNode)
        {
            continue;  // sibling-slice relationId, not in this tree
        }

        // Capture parentId before mutating the tree.
        std::string oldParentId;
        if (auto p = std::dynamic_pointer_cast<ContactTreeNode>(childNode->getParent().lock()))
        {
            if (p != m_root)
            {
                oldParentId = p->getNodeData().id;
            }
        }

        detachFromParent(childNode);
        childNode->setRelationId({});
        m_root->addChild(childNode);

        removed.push_back({rid, std::move(childId), std::move(oldParentId)});
    }
    return removed;
}

}
