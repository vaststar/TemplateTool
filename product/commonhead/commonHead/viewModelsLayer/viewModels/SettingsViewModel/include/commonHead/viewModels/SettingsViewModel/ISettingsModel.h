#pragma once
#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>
#include <AssetToken.h>

#include <string>
#include <memory>
#include <cstdint>

namespace commonHead::viewModels::model {

/**
 * @brief Describes a single structural change (insert/remove) in the settings tree.
 */
struct COMMONHEAD_EXPORT SettingsTreeNodeChange
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
 * @brief Settings panel type - UI layer maps to specific QML file
 */
enum class COMMONHEAD_EXPORT SettingsPanelType : uint8_t
{
    None = 0,           // No panel (category only)
    Appearance,         // Appearance settings
    Language,           // Language settings
};

/**
 * @brief Settings node data
 */
struct COMMONHEAD_EXPORT SettingsNodeData
{
    std::string nodeId;
    std::string title;
    commonHead::model::AssetImageToken icon = commonHead::model::AssetImageToken::None;
    SettingsPanelType panelType = SettingsPanelType::None;
};

/**
 * @brief Settings tree node interface
 */
class COMMONHEAD_EXPORT ISettingsTreeNode
{
public:
    virtual ~ISettingsTreeNode() = default;

    virtual SettingsNodeData getNodeData() const = 0;
    virtual void setNodeData(const SettingsNodeData& data) = 0;

    virtual std::weak_ptr<ISettingsTreeNode> getParent() const = 0;

    virtual std::size_t getChildCount() const = 0;

    virtual std::shared_ptr<ISettingsTreeNode> getChild(std::size_t index) const = 0;
};

using SettingsTreeNodePtr = std::shared_ptr<ISettingsTreeNode>;

/**
 * @brief Settings tree interface
 */
class COMMONHEAD_EXPORT ISettingsTree
{
public:
    virtual ~ISettingsTree() = default;

    virtual SettingsTreeNodePtr getRoot() const = 0;

    virtual SettingsTreeNodePtr findNodeById(const std::string& nodeId) const = 0;

    virtual bool removeNode(const std::string& nodeId) = 0;
};

using SettingsTreePtr = std::shared_ptr<ISettingsTree>;

} // namespace commonHead::viewModels::model
