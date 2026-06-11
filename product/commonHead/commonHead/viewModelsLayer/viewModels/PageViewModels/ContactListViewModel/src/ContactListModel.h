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
    void removeChild(const std::string& childId);
    void updateData(const ContactNodeData& data);
    // Move all children out (used when re-parenting orphans to virtual root)
    std::vector<std::shared_ptr<ContactTreeNode>> takeChildren();

    // Identity of the relation row that attaches this node to its parent in the slice
    // this tree represents. Empty when the node is at the virtual root.
    const std::string& getRelationId() const;
    void               setRelationId(std::string relationId);
private:
    ContactNodeData m_data;
    std::weak_ptr<ContactTreeNode> m_parent;
    std::vector<std::shared_ptr<ContactTreeNode>> m_children;
    std::unordered_map<std::string, std::size_t> m_childIndex; // id -> position in m_children
    std::string m_relationId; // service-layer relation row id; empty => attached to virtual root
};


class ContactTree : public IContactTree
{
public:
    ContactTree(const std::vector<ucf::service::model::IContactPtr>& contacts, const std::vector<ucf::service::model::IContactRelationPtr>& relations);

    ContactTreeNodePtr getRoot() const override;

    ContactTreeNodePtr findNodeById(const std::string& id) const override;

    // Incremental mutations (single)
    void addNode(const ContactNodeData& data);
    void updateNode(const ContactNodeData& data);
    void removeNode(const std::string& id);
    void setRelation(const ContactRelationData& relation);

    // Batch variants (preferred for callback payloads)
    void addNodes(const std::vector<ContactNodeData>& datas);
    void updateNodes(const std::vector<ContactNodeData>& datas);
    void removeNodes(const std::vector<std::string>& ids);
    void setRelations(const std::vector<ContactRelationData>& relations);

    // ===== Relation row identity lookups =====
    // Each child has at most one relation row in this tree (the slice the VM owns).

    // Returns the relationId associated with childId, or empty if the child is at the
    // virtual root (no parent relation row).
    std::string getRelationIdByChildId(const std::string& childId) const;

    // Resolves each relationId to its child, captures the pre-detach parentId, detaches
    // the node, clears its relationId, and re-parents it under the virtual root. Returns
    // one entry per relationId that matched a node in this tree; relationIds belonging
    // to sibling slices (not found here) are silently skipped.
    std::vector<RemovedRelationInfo>
    removeRelationsByIds(const std::vector<std::string>& relationIds);

private:
    void buildNodes(const std::vector<ucf::service::model::IContactPtr>& contacts);
    void buildRelations(const std::vector<ucf::service::model::IContactRelationPtr>& relations);
    void createVirtualRoot();
    void detachFromParent(const std::shared_ptr<ContactTreeNode>& node);
private:
    std::shared_ptr<ContactTreeNode> m_root;
    std::unordered_map<std::string, std::shared_ptr<ContactTreeNode>> m_nodes;
};

}
