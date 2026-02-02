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

struct COMMONHEAD_EXPORT ContactNodeData
{
    std::string id;
    std::string displayName;
    ContactNodeType type;
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
