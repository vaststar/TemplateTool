#pragma once
#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>

#include <string>
#include <vector>
#include <memory>

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
    Department = 0,
    Project    = 1,
    Team       = 2,
    Custom     = 3,
};

// Mirrors ucf::service::model::IContactRelation::RelationType.
enum class COMMONHEAD_EXPORT RelationType
{
    Department    = 0,
    Reporting     = 1,
    Project       = 2,
    Mentor        = 3,
    Collaboration = 4,
};

struct COMMONHEAD_EXPORT ContactNodeData
{
    std::string     id;
    std::string     displayName;
    ContactNodeType type;
    // Only meaningful when type == Group. For Person nodes the value is unused
    // (default-initialized to Department).
    GroupType       groupType{GroupType::Department};
};

struct COMMONHEAD_EXPORT ContactRelationData
{
    // Surrogate primary key (UUID). Empty when callers issue an "add" request and
    // expect the service layer to mint a new id.
    std::string  id;
    std::string  parentId;
    std::string  childId;
    RelationType type{RelationType::Department};
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
