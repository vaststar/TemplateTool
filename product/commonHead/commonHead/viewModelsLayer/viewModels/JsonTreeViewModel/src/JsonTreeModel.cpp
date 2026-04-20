#include "JsonTreeModel.h"

#include <ucf/Utilities/JsonUtils/JsonValue.h>

#include <sstream>

namespace commonHead::viewModels::model {

// ============== JsonTreeNode ==============

JsonTreeNode::JsonTreeNode(const JsonNodeData& data)
    : m_data(data)
{
}

JsonNodeData JsonTreeNode::getNodeData() const
{
    return m_data;
}

std::weak_ptr<IJsonTreeNode> JsonTreeNode::getParent() const
{
    if (auto p = m_parent.lock())
    {
        return std::static_pointer_cast<IJsonTreeNode>(p);
    }
    return std::weak_ptr<IJsonTreeNode>();
}

std::size_t JsonTreeNode::getChildCount() const
{
    return m_children.size();
}

std::shared_ptr<IJsonTreeNode> JsonTreeNode::getChild(std::size_t index) const
{
    if (index < m_children.size())
    {
        return std::static_pointer_cast<IJsonTreeNode>(m_children[index]);
    }
    return nullptr;
}

void JsonTreeNode::setParent(const std::shared_ptr<JsonTreeNode>& parent)
{
    m_parent = parent;
}

void JsonTreeNode::addChild(const std::shared_ptr<JsonTreeNode>& child)
{
    if (!child)
    {
        return;
    }
    child->setParent(shared_from_this());
    m_children.push_back(child);
}

// ============== JsonTree ==============

JsonTreeNodePtr JsonTree::getRoot() const
{
    return std::static_pointer_cast<IJsonTreeNode>(m_root);
}

std::string JsonTree::getJsonPath(const IJsonTreeNode* node) const
{
    if (!node)
    {
        return "$";
    }

    // Collect ancestors
    std::vector<const IJsonTreeNode*> ancestors;
    const IJsonTreeNode* current = node;
    while (current)
    {
        ancestors.push_back(current);
        auto parentWeak = current->getParent();
        auto parentShared = parentWeak.lock();
        if (!parentShared || parentShared.get() == m_root.get())
        {
            break;
        }
        current = parentShared.get();
    }

    // Build path from root to node
    std::string path = "$";
    // ancestors is [node, ..., child_of_root], reverse to get top-down
    for (auto it = ancestors.rbegin(); it != ancestors.rend(); ++it)
    {
        auto data = (*it)->getNodeData();
        if (!data.key.empty())
        {
            // Object member
            path += "." + data.key;
        }
        else if (it != ancestors.rbegin())
        {
            // Array element — find index in parent
            auto parentIt = it;
            --parentIt; // parentIt is closer to root since we reversed
            // Actually we need the parent of *it
            auto parentWeak = (*it)->getParent();
            auto parentShared = parentWeak.lock();
            if (parentShared && parentShared->getNodeData().type == JsonNodeType::Array)
            {
                for (std::size_t i = 0; i < parentShared->getChildCount(); ++i)
                {
                    if (parentShared->getChild(i).get() == *it)
                    {
                        path += "[" + std::to_string(i) + "]";
                        break;
                    }
                }
            }
        }
    }

    return path;
}

std::string JsonTree::getSubTreeJson(const IJsonTreeNode* node, int indent) const
{
    if (!node)
    {
        return {};
    }

    auto data = node->getNodeData();

    switch (data.type)
    {
    case JsonNodeType::Object:
    case JsonNodeType::Array:
    {
        // Reconstruct the JsonValue subtree from the node and dump
        // For simplicity, we rebuild via recursive walk
        std::function<ucf::utilities::JsonValue(const IJsonTreeNode*)> rebuild;
        rebuild = [&rebuild](const IJsonTreeNode* n) -> ucf::utilities::JsonValue {
            auto d = n->getNodeData();
            switch (d.type)
            {
            case JsonNodeType::Object:
            {
                auto obj = ucf::utilities::JsonValue::object();
                for (std::size_t i = 0; i < n->getChildCount(); ++i)
                {
                    auto child = n->getChild(i);
                    auto childData = child->getNodeData();
                    if (childData.type == JsonNodeType::ClosingBracket)
                        continue;
                    obj.set(childData.key, rebuild(child.get()));
                }
                return obj;
            }
            case JsonNodeType::Array:
            {
                auto arr = ucf::utilities::JsonValue::array();
                for (std::size_t i = 0; i < n->getChildCount(); ++i)
                {
                    auto child = n->getChild(i);
                    if (child->getNodeData().type == JsonNodeType::ClosingBracket)
                        continue;
                    arr.push_back(rebuild(child.get()));
                }
                return arr;
            }
            case JsonNodeType::String:
                return ucf::utilities::JsonValue(d.displayValue);
            case JsonNodeType::Number:
            {
                // Try integer first, then double
                auto parsed = ucf::utilities::JsonValue::parse(d.displayValue);
                if (!parsed.isNull())
                {
                    return parsed;
                }
                return ucf::utilities::JsonValue(0.0);
            }
            case JsonNodeType::Bool:
                return ucf::utilities::JsonValue(d.displayValue == "true");
            case JsonNodeType::Null:
            default:
                return ucf::utilities::JsonValue(nullptr);
            }
        };

        auto value = rebuild(node);
        return value.dumpPretty(indent);
    }
    case JsonNodeType::String:
        return "\"" + data.displayValue + "\"";
    default:
        return data.displayValue;
    }
}

std::shared_ptr<JsonTree> JsonTree::build(const std::string& jsonStr, std::string& error)
{
    auto parseResult = ucf::utilities::JsonValue::parseEx(jsonStr);
    if (!parseResult.ok())
    {
        error = parseResult.error;
        return nullptr;
    }

    // Create invisible virtual root; the actual JSON node becomes its child
    // so that the JSON root (Object/Array) is visible and collapsible in the UI
    JsonNodeData virtualRootData;
    virtualRootData.type = JsonNodeType::Null;
    auto virtualRoot = std::make_shared<JsonTreeNode>(virtualRootData);

    auto jsonNode = buildNode(parseResult.value, "");
    virtualRoot->addChild(jsonNode);

    auto tree = std::make_shared<JsonTree>();
    tree->m_root = virtualRoot;
    return tree;
}

std::shared_ptr<JsonTreeNode> JsonTree::buildNode(
    const ucf::utilities::JsonValue& value,
    const std::string& key)
{
    JsonNodeData data;
    data.key = key;

    if (value.isObject())
    {
        data.type = JsonNodeType::Object;
        data.childCount = value.size();
        auto node = std::make_shared<JsonTreeNode>(data);

        for (auto [childKey, childValue] : value.items())
        {
            auto childNode = buildNode(childValue, childKey);
            node->addChild(childNode);
        }

        // Synthetic closing bracket node
        JsonNodeData closingData;
        closingData.type = JsonNodeType::ClosingBracket;
        closingData.displayValue = "}";
        node->addChild(std::make_shared<JsonTreeNode>(closingData));

        return node;
    }
    else if (value.isArray())
    {
        data.type = JsonNodeType::Array;
        data.childCount = value.size();
        auto node = std::make_shared<JsonTreeNode>(data);

        std::size_t index = 0;
        for (auto it = value.begin(); it != value.end(); ++it, ++index)
        {
            auto childNode = buildNode(*it, "");
            node->addChild(childNode);
        }

        // Synthetic closing bracket node
        JsonNodeData closingData;
        closingData.type = JsonNodeType::ClosingBracket;
        closingData.displayValue = "]";
        node->addChild(std::make_shared<JsonTreeNode>(closingData));

        return node;
    }
    else if (value.isString())
    {
        data.type = JsonNodeType::String;
        data.displayValue = value.asString().value_or("");
    }
    else if (value.isNumber())
    {
        data.type = JsonNodeType::Number;
        if (value.isInteger())
        {
            data.displayValue = std::to_string(value.asInt64().value_or(0));
        }
        else
        {
            // Use dump() to get clean number representation
            data.displayValue = value.dump();
        }
    }
    else if (value.isBool())
    {
        data.type = JsonNodeType::Bool;
        data.displayValue = value.asBool().value_or(false) ? "true" : "false";
    }
    else
    {
        data.type = JsonNodeType::Null;
        data.displayValue = "null";
    }

    return std::make_shared<JsonTreeNode>(data);
}

} // namespace commonHead::viewModels::model
