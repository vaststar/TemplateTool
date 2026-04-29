
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
    child->setParent(shared_from_this());
    m_children.push_back(child);
}


// ---------------- ContactTree ----------------

ContactTree::ContactTree(const std::vector<ucf::service::model::IContactPtr>& contacts, const std::vector<ucf::service::model::IContactRelationPtr>& relations)
{
    buildNodes(contacts);

    buildRelations(relations);

    createVirtualRoot();

    m_index.clear();
    m_index.reserve(m_nodes.size());
    for (const auto& [id, node] : m_nodes)
    {
        m_index.emplace(id, node);
    }
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
        if (itParent == m_nodes.end())
        {
            continue;
        }
        auto parentNode = itParent->second;
        if (!parentNode) {
            continue;
        }

        const std::string childId  = rel->getChildId();
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

ContactTreeNodePtr ContactTree::getRoot() const
{
    return std::static_pointer_cast<IContactTreeNode>(m_root);
}

ContactTreeNodePtr ContactTree::findNodeById(const std::string& id) const
{
    
    if (auto it = m_index.find(id); it != m_index.end())
    {
        if (auto ptr = it->second.lock())
        {
            return std::static_pointer_cast<IContactTreeNode>(ptr);
        }
    }
    return nullptr;
}

}