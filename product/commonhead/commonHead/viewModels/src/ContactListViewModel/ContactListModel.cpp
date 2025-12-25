
#include "ContactListModel.h"

#include <algorithm>
#include <stdexcept>

#include <ucf/Services/ContactService/IContactEntities.h>

namespace commonHead::viewModels::model{


// ---------------- ContactTreeNode ----------------

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
    // 向上转型到接口类型
    std::weak_ptr<IContactTreeNode> result;
    if (auto p = m_parent.lock()) {
        result = std::static_pointer_cast<IContactTreeNode>(p);
    }
    return result;
}

std::size_t ContactTreeNode::getChildCount() const
{
    return m_children.size();
}

std::shared_ptr<IContactTreeNode> ContactTreeNode::getChild(std::size_t index) const
{
    if (index >= m_children.size()) {
        return nullptr;
    }
    return std::static_pointer_cast<IContactTreeNode>(m_children[index]);
}

void ContactTreeNode::setParent(const std::shared_ptr<ContactTreeNode>& parent)
{
    m_parent = parent;
}

void ContactTreeNode::addChild(const std::shared_ptr<ContactTreeNode>& child)
{
    if (!child) {
        return;
    }
    child->setParent(shared_from_this());
    m_children.push_back(child);
}


// ---------------- ContactTree ----------------

ContactTree::ContactTree(
    const std::vector<ucf::service::model::IContactPtr>& contacts,
    const std::vector<ucf::service::model::IContactRelationPtr>& relations)
{
    // 1. 创建所有节点（id -> ContactTreeNode）
    buildNodes(contacts);

    // 2. 根据关系建立 parent-child
    buildRelations(relations);

    // 3. 找出所有 root 节点（没有 parent 的节点），挂到虚拟根下
    std::unordered_set<std::string> rootIds;
    rootIds.reserve(m_nodes.size());
    for (const auto& kv : m_nodes) {
        rootIds.insert(kv.first);
    }

    // 遍历所有有 parent 的 child，把它们从 root 候选里移除
    for (const auto& kv : m_nodes) {
        const auto& node = kv.second;
        if (!node) {
            continue;
        }
        if (auto parent = node->getParent().lock()) {
            rootIds.erase(node->getNodeData().id);
        }
    }

    // 创建虚拟根，并将所有根节点挂在虚拟根下面
    m_root = createVirtualRoot(rootIds);

    // 4. 构建索引（id -> weak_ptr）
    m_index.clear();
    m_index.reserve(m_nodes.size());
    for (const auto& kv : m_nodes) {
        m_index.emplace(kv.first, kv.second);
    }
}

ContactTree::NodeImplPtr ContactTree::createVirtualRoot(const std::unordered_set<std::string>& rootIds)
{
    ContactNodeData rootData;
    rootData.id = "";                // 虚拟根 id，可按需自定义
    rootData.displayName = "";       // UI 层可以选择不显示根
    rootData.type = ContactNodeType::Group;

    auto root = std::make_shared<ContactTreeNode>(rootData);

    // 所有 rootIds 对应的节点挂在虚拟根下面
    for (const auto& id : rootIds) {
        auto it = m_nodes.find(id);
        if (it == m_nodes.end()) {
            continue;
        }
        const auto& child = it->second;
        if (child) {
            root->addChild(child);
        }
    }

    return root;
}

void ContactTree::buildNodes(const std::vector<ucf::service::model::IContactPtr>& contacts)
{
    m_nodes.clear();
    m_nodes.reserve(contacts.size());

    for (const auto& c : contacts) {
        if (!c) {
            continue;
        }
        const std::string id = c->getContactId();
        if (id.empty()) {
            continue;
        }

        ContactNodeData data;
        data.id = id;

        // 判断是 Person 还是 Group，并设置显示名
        // 这里尝试 dynamic_pointer_cast 到 IPersonContact / IGroupContact
        if (auto person = std::dynamic_pointer_cast<ucf::service::model::IPersonContact>(c)) {
            data.type = ContactNodeType::Person;
            data.displayName = person->getPersonName();
        } else if (auto group = std::dynamic_pointer_cast<ucf::service::model::IGroupContact>(c)) {
            data.type = ContactNodeType::Group;
            data.displayName = group->getGroupName();
        } else {
            // fallback：如果类型未知，就当 Person，用 id 作为名字
            data.type = ContactNodeType::Person;
            data.displayName = id;
        }

        auto node = std::make_shared<ContactTreeNode>(data);
        m_nodes.emplace(id, node);
    }
}

void ContactTree::buildRelations(const std::vector<ucf::service::model::IContactRelationPtr>& relations)
{
    for (const auto& rel : relations) {
        if (!rel) {
            continue;
        }

        const std::string childId  = rel->getChildId();
        const std::string parentId = rel->getParentId();

        // childId 必须存在
        auto itChild = m_nodes.find(childId);
        if (itChild == m_nodes.end()) {
            continue;
        }
        auto childNode = itChild->second;

        // parentId 为空：表示这是一个顶层节点，先不处理，后面统一挂到虚拟根
        if (parentId.empty()) {
            continue;
        }

        auto itParent = m_nodes.find(parentId);
        if (itParent == m_nodes.end()) {
            // 找不到 parent，直接略过这条关系
            continue;
        }

        auto parentNode = itParent->second;
        if (!parentNode) {
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
    auto it = m_index.find(id);
    if (it == m_index.end()) {
        return nullptr;
    }
    auto ptr = it->second.lock();
    return std::static_pointer_cast<IContactTreeNode>(ptr);
}

}