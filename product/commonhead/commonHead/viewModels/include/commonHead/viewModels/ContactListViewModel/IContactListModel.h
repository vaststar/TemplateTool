#pragma once
#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>

#include <string>
#include <vector>
#include <memory>

namespace commonHead::viewModels::model{
// 节点类型（区分人/组，不暴露 Service 层实现）
enum class COMMONHEAD_EXPORT ContactNodeType
{
    Person,
    Group
};

// 供 UI 或中间层访问的节点数据（完全脱离 Service 层）
struct COMMONHEAD_EXPORT ContactNodeData
{
    std::string id;           // contactId (来自 Service 但不依赖 service 类型)
    std::string displayName;  // 名字/组名
    ContactNodeType type;     // Person / Group
};

// 单个节点的数据接口（树节点）
class COMMONHEAD_EXPORT IContactTreeNode
{
public:
    virtual ~IContactTreeNode() = default;

    // 提供节点的基本静态数据
    virtual ContactNodeData getNodeData() const = 0;

    // 返回父节点（weak_ptr 完美避免循环引用）
    virtual std::weak_ptr<IContactTreeNode> getParent() const = 0;

    // 子节点数量
    virtual std::size_t getChildCount() const = 0;

    // 获取某个子节点（shared_ptr 让 UI 安全持有）
    virtual std::shared_ptr<IContactTreeNode> getChild(std::size_t index) const = 0;
};

using ContactTreeNodePtr = std::shared_ptr<IContactTreeNode>;

// 整棵树的数据接口
class COMMONHEAD_EXPORT IContactTree
{
public:
    virtual ~IContactTree() = default;

    // 获取根节点
    virtual ContactTreeNodePtr getRoot() const = 0;

    // 用 id 查找节点（常用）
    virtual ContactTreeNodePtr findNodeById(const std::string& id) const = 0;
};

using ContactTreePtr = std::shared_ptr<IContactTree>;
}
