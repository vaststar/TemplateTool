#pragma once
#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace commonHead::viewModels::model{
enum class COMMONHEAD_EXPORT ContactNodeType
{
    Person,
    Group
};

// Mirrors ucf::service::model::IGroupContact::GroupType. Kept as a separate enum so the
// VM/UI layer does not have to depend on service headers; converters in utils translate
// across the boundary.
enum class COMMONHEAD_EXPORT GroupType
{
    Folder     = 0,
    Department = 1,
    Project    = 2,
    Team       = 3,
    Custom     = 4,
};

// Mirrors ucf::service::model::IContactRelation::RelationType.
enum class COMMONHEAD_EXPORT RelationType
{
    Folder        = 0,
    Department    = 1,
    Reporting     = 2,
    Project       = 3,
    Mentor        = 4,
    Collaboration = 5,
};

struct COMMONHEAD_EXPORT ContactNodeData
{
    std::string     id;
    std::string     displayName;
    ContactNodeType type;
    // Only meaningful when type == Group. For Person nodes the value is unused
    // (default-initialized to Folder).
    GroupType       groupType{GroupType::Folder};
};

struct COMMONHEAD_EXPORT ContactRelationData
{
    // Surrogate primary key (UUID). Empty when callers issue an "add" request and
    // expect the service layer to mint a new id.
    std::string  id;
    std::string  parentId;
    std::string  childId;
    RelationType type{RelationType::Folder};
};

// Returned by ContactTree::removeRelationsByIds. Carries the ids the tree needed to
// re-locate the node, plus the parentId captured before the detach so downstream
// consumers (UI item models, callback payloads, ...) can describe the edge that was
// removed without having to query a now-mutated tree.
struct COMMONHEAD_EXPORT RemovedRelationInfo
{
    std::string relationId;
    std::string childId;
    std::string oldParentId;  // empty if the child was at the virtual root
};

// ===== Detail-view payload =====
// The tree model only carries id/displayName/type so it stays cheap to broadcast. The
// detail panel needs richer fields; ContactDetail is the one-shot payload returned by
// IContactListViewModel::getContactDetail for a single contact.

enum class COMMONHEAD_EXPORT ContactStatus
{
    Active   = 0,
    Inactive = 1,
    Deleted  = 2,
    Archived = 3,
};

enum class COMMONHEAD_EXPORT Gender
{
    Unspecified = 0,
    Male        = 1,
    Female      = 2,
    Other       = 3,
};

struct COMMONHEAD_EXPORT PersonContactDetail
{
    std::string firstName;
    std::string lastName;
    Gender      gender{Gender::Unspecified};
    std::string phone;
    std::string email;
};

// Sub-row of a Department-typed group. managerDisplayName is a best-effort lookup
// against the person directory; empty when the manager id does not resolve.
struct COMMONHEAD_EXPORT DepartmentGroupDetail
{
    std::string managerId;
    std::string managerDisplayName;
    int         headcount{0};
};

// Sub-row of a Team-typed group. teamLeadDisplayName is best-effort, same as above.
struct COMMONHEAD_EXPORT TeamGroupDetail
{
    std::string teamLeadId;
    std::string teamLeadDisplayName;
    std::string mission;
};

// Variant-style aggregate. (type, groupType) selects which optional is engaged:
//   type==Person                                 -> person
//   type==Group  + groupType==Department         -> department
//   type==Group  + groupType==Team               -> team
//   type==Group  + groupType==Folder/Project/Custom -> none (base info only)
struct COMMONHEAD_EXPORT ContactDetail
{
    std::string     id;
    std::string     displayName;
    ContactNodeType type{ContactNodeType::Person};
    GroupType       groupType{GroupType::Folder};
    ContactStatus   status{ContactStatus::Active};

    std::optional<PersonContactDetail>    person;
    std::optional<DepartmentGroupDetail>  department;
    std::optional<TeamGroupDetail>        team;
};

enum class COMMONHEAD_EXPORT ContactDirectoryLoadError
{
    Unknown,
    DatabaseNotBound,
    DatabaseReadFailed,
};

class COMMONHEAD_EXPORT IContactTreeNode
{
public:
    virtual ~IContactTreeNode() = default;

    virtual ContactNodeData getNodeData() const = 0;

    virtual std::weak_ptr<IContactTreeNode> getParent() const = 0;

    virtual std::size_t getChildCount() const = 0;

    virtual std::shared_ptr<IContactTreeNode> getChild(std::size_t index) const = 0;
};

using ContactTreeNodePtr = std::shared_ptr<IContactTreeNode>;

class COMMONHEAD_EXPORT IContactTree
{
public:
    virtual ~IContactTree() = default;

    virtual ContactTreeNodePtr getRoot() const = 0;

    virtual ContactTreeNodePtr findNodeById(const std::string& id) const = 0;
};

using ContactTreePtr = std::shared_ptr<IContactTree>;
}
