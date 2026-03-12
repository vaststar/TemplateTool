#pragma once

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>

#include <string>
#include <memory>
#include <cstdint>

namespace commonHead::viewModels::model {

/**
 * @brief Describes a single structural change (insert/remove) in the tools tree.
 */
struct COMMONHEAD_EXPORT ToolsTreeNodeChange
{
    enum class Type : uint8_t
    {
        Inserted,   ///< A new node was added
        Removed,    ///< An existing node was removed
    };

    Type type;
    std::string parentNodeId;   ///< Parent node that owns the affected child
    std::string nodeId;         ///< The inserted / removed node ID
    std::size_t index;          ///< Row position within the parent's children
};

/**
 * @brief 工具面板类型 - UI 层映射到具体 QML 文件
 */
enum class COMMONHEAD_EXPORT ToolPanelType : uint8_t
{
    None = 0,           // 无面板（分类节点）
    Base64,             // Base64 编解码
    Json,               // JSON 格式化
    Timestamp,          // 时间戳转换
    Uuid,               // UUID 生成
};

/**
 * @brief 工具节点数据
 */
struct COMMONHEAD_EXPORT ToolNodeData
{
    std::string nodeId;
    std::string title;
    std::string icon;
    ToolPanelType panelType = ToolPanelType::None;
};

/**
 * @brief 工具树节点接口
 */
class COMMONHEAD_EXPORT IToolsTreeNode
{
public:
    virtual ~IToolsTreeNode() = default;

    virtual ToolNodeData getNodeData() const = 0;
    virtual void setNodeData(const ToolNodeData& data) = 0;

    virtual std::weak_ptr<IToolsTreeNode> getParent() const = 0;

    virtual std::size_t getChildCount() const = 0;

    virtual std::shared_ptr<IToolsTreeNode> getChild(std::size_t index) const = 0;
};

using ToolsTreeNodePtr = std::shared_ptr<IToolsTreeNode>;

/**
 * @brief 工具树接口
 */
class COMMONHEAD_EXPORT IToolsTree
{
public:
    virtual ~IToolsTree() = default;

    virtual ToolsTreeNodePtr getRoot() const = 0;

    virtual ToolsTreeNodePtr findNodeById(const std::string& nodeId) const = 0;

    virtual bool removeNode(const std::string& nodeId) = 0;
};

using ToolsTreePtr = std::shared_ptr<IToolsTree>;

} // namespace commonHead::viewModels::model
