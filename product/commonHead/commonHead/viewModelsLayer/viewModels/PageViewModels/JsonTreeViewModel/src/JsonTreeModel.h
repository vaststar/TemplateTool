#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <commonHead/viewModels/JsonTreeViewModel/IJsonTreeModel.h>

namespace ucf::utilities {
class JsonValue;
}

namespace commonHead::viewModels::model {

class JsonTreeNode : public IJsonTreeNode,
                     public std::enable_shared_from_this<JsonTreeNode>
{
public:
    explicit JsonTreeNode(const JsonNodeData& data);

    JsonNodeData getNodeData() const override;
    std::weak_ptr<IJsonTreeNode> getParent() const override;
    std::size_t getChildCount() const override;
    std::shared_ptr<IJsonTreeNode> getChild(std::size_t index) const override;

    void setParent(const std::shared_ptr<JsonTreeNode>& parent);
    void addChild(const std::shared_ptr<JsonTreeNode>& child);

private:
    JsonNodeData m_data;
    std::weak_ptr<JsonTreeNode> m_parent;
    std::vector<std::shared_ptr<JsonTreeNode>> m_children;
};

class JsonTree : public IJsonTree
{
public:
    JsonTree() = default;

    JsonTreeNodePtr getRoot() const override;
    std::string getJsonPath(const IJsonTreeNode* node) const override;
    std::string getSubTreeJson(const IJsonTreeNode* node, int indent = 2) const override;

    /**
     * @brief 从 JSON 字符串构建树
     * @param jsonStr JSON 字符串
     * @param[out] error 解析失败时写入错误信息
     * @return 成功返回 JsonTree 实例，失败返回 nullptr
     */
    static std::shared_ptr<JsonTree> build(const std::string& jsonStr, std::string& error);

private:
    static std::shared_ptr<JsonTreeNode> buildNode(
        const ucf::utilities::JsonValue& value,
        const std::string& key);

    std::shared_ptr<JsonTreeNode> m_root;
};

} // namespace commonHead::viewModels::model
