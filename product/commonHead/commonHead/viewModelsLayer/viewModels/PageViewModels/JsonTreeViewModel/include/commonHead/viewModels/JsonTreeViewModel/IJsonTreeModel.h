#pragma once

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>

#include <cstdint>
#include <memory>
#include <string>

namespace commonHead::viewModels::model {

/**
 * @brief JSON 节点类型
 */
enum class COMMONHEAD_EXPORT JsonNodeType : uint8_t
{
    Object,         // 0
    Array,          // 1
    String,         // 2
    Number,         // 3
    Bool,           // 4
    Null,           // 5
    ClosingBracket  // 6 — synthetic "}" or "]" row
};

/**
 * @brief JSON 树节点数据
 */
struct COMMONHEAD_EXPORT JsonNodeData
{
    JsonNodeType type = JsonNodeType::Null;
    std::string key;            ///< 对象成员的 key，数组元素为空
    std::string displayValue;   ///< 叶子节点的显示值（如 "hello"、42、true、null）
    std::size_t childCount = 0; ///< Object/Array 的子节点数量，用于折叠时显示
};

/**
 * @brief JSON 树节点接口
 */
class COMMONHEAD_EXPORT IJsonTreeNode
{
public:
    virtual ~IJsonTreeNode() = default;

    virtual JsonNodeData getNodeData() const = 0;

    virtual std::weak_ptr<IJsonTreeNode> getParent() const = 0;

    virtual std::size_t getChildCount() const = 0;

    virtual std::shared_ptr<IJsonTreeNode> getChild(std::size_t index) const = 0;
};

using JsonTreeNodePtr = std::shared_ptr<IJsonTreeNode>;

/**
 * @brief JSON 树接口
 */
class COMMONHEAD_EXPORT IJsonTree
{
public:
    virtual ~IJsonTree() = default;

    virtual JsonTreeNodePtr getRoot() const = 0;

    /**
     * @brief 获取节点对应的 JSON Path（如 $.data.users[0].name）
     */
    virtual std::string getJsonPath(const IJsonTreeNode* node) const = 0;

    /**
     * @brief 获取节点对应的子树 JSON 字符串
     */
    virtual std::string getSubTreeJson(const IJsonTreeNode* node, int indent = 2) const = 0;
};

using JsonTreePtr = std::shared_ptr<IJsonTree>;

} // namespace commonHead::viewModels::model
