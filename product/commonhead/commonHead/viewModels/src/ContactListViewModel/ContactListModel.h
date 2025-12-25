#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <ucf/Services/ContactService/IContactEntities.h>
#include <commonHead/viewModels/ContactListViewModel/IContactListModel.h>

namespace ucf::service::model{
    class IPersonContact;
    using IPersonContactPtr = std::shared_ptr<IPersonContact>;

    class IGroupContact;
    using IGroupContactPtr = std::shared_ptr<IGroupContact>;

    class IContactRelation;
    using IContactRelationPtr = std::shared_ptr<IContactRelation>;
}
namespace commonHead::viewModels::model{
// 前向声明：内部实现类
class ContactTreeNode;
class ContactTree;

// 具体节点实现：实现 IContactTreeNode
class ContactTreeNode : public IContactTreeNode,
                        public std::enable_shared_from_this<ContactTreeNode>
{
public:
    explicit ContactTreeNode(const ContactNodeData& data);

    ContactNodeData getNodeData() const override;

    std::weak_ptr<IContactTreeNode> getParent() const override;

    std::size_t getChildCount() const override;

    std::shared_ptr<IContactTreeNode> getChild(std::size_t index) const override;

    // 供 ContactTree 使用的内部接口
    void setParent(const std::shared_ptr<ContactTreeNode>& parent);
    void addChild(const std::shared_ptr<ContactTreeNode>& child);

private:
    ContactNodeData m_data;
    std::weak_ptr<ContactTreeNode> m_parent;
    std::vector<std::shared_ptr<ContactTreeNode>> m_children;
};


// 整棵树实现：实现 IContactTree
// 构造函数从 Service 层的联系人 + 关系来拼树
class ContactTree : public IContactTree
{
public:
    // contacts: 所有 IContact（人 + 组）
    // relations: 所有父子关系（childId / parentId）
    ContactTree(
        const std::vector<ucf::service::model::IContactPtr>& contacts,
        const std::vector<ucf::service::model::IContactRelationPtr>& relations);

    ContactTreeNodePtr getRoot() const override;

    ContactTreeNodePtr findNodeById(const std::string& id) const override;

private:
    using NodeImplPtr = std::shared_ptr<ContactTreeNode>;

    void buildNodes(const std::vector<ucf::service::model::IContactPtr>& contacts);
    void buildRelations(const std::vector<ucf::service::model::IContactRelationPtr>& relations);

    NodeImplPtr createVirtualRoot(const std::unordered_set<std::string>& rootIds);

private:
    NodeImplPtr m_root;  // 可能是一个“虚拟根”
    std::unordered_map<std::string, std::weak_ptr<ContactTreeNode>> m_index; // id -> node
    std::unordered_map<std::string, NodeImplPtr> m_nodes; // 构建过程中的强引用
};

}