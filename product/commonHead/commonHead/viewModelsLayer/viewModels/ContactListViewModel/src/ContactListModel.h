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
class ContactTreeNode;
class ContactTree;

class ContactTreeNode : public IContactTreeNode,
                        public std::enable_shared_from_this<ContactTreeNode>
{
public:
    explicit ContactTreeNode(const ContactNodeData& data);

    ContactNodeData getNodeData() const override;

    std::weak_ptr<IContactTreeNode> getParent() const override;

    std::size_t getChildCount() const override;

    std::shared_ptr<IContactTreeNode> getChild(std::size_t index) const override;

    void setParent(const std::shared_ptr<ContactTreeNode>& parent);
    void addChild(const std::shared_ptr<ContactTreeNode>& child);
private:
    ContactNodeData m_data;
    std::weak_ptr<ContactTreeNode> m_parent;
    std::vector<std::shared_ptr<ContactTreeNode>> m_children;
};


class ContactTree : public IContactTree
{
public:
    ContactTree(const std::vector<ucf::service::model::IContactPtr>& contacts, const std::vector<ucf::service::model::IContactRelationPtr>& relations);

    ContactTreeNodePtr getRoot() const override;

    ContactTreeNodePtr findNodeById(const std::string& id) const override;

private:
    void buildNodes(const std::vector<ucf::service::model::IContactPtr>& contacts);
    void buildRelations(const std::vector<ucf::service::model::IContactRelationPtr>& relations);
    void createVirtualRoot();
private:
    std::shared_ptr<ContactTreeNode> m_root;
    std::unordered_map<std::string, std::weak_ptr<ContactTreeNode>> m_index; //just for indexing
    std::unordered_map<std::string, std::shared_ptr<ContactTreeNode>> m_nodes; // actual tree nodes
};

}