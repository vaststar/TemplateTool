#pragma once

#include <AssetToken.h>

#include <cstdint>
#include <memory>
#include <string>

#include <commonHead/viewModels/SettingsViewModel/SettingsViewModelTypesExport.h>

namespace commonHead::viewModels::model {

/**
 * @brief Settings panel type - UI layer maps to specific QML file
 */
enum class SettingsPanelType : std::uint8_t
{
    None = 0,           // No panel (category only)
    Appearance,         // Appearance settings
    Language,           // Language settings
};

/**
 * @brief Settings node data. parentId == "" means the node hangs off the virtual root.
 */
struct SETTINGS_VIEW_MODEL_TYPES_API SettingsNodeData
{
    std::string nodeId;
    std::string parentId;
    std::string title;
    commonHead::model::AssetImageToken icon = commonHead::model::AssetImageToken::None;
    SettingsPanelType panelType = SettingsPanelType::None;
};

/**
 * @brief Settings tree node interface
 */
class SETTINGS_VIEW_MODEL_TYPES_API ISettingsTreeNode
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
class SETTINGS_VIEW_MODEL_TYPES_API ISettingsTree
{
public:
    virtual ~ISettingsTree() = default;

    virtual SettingsTreeNodePtr getRoot() const = 0;

    virtual SettingsTreeNodePtr findNodeById(const std::string& nodeId) const = 0;

    virtual bool removeNode(const std::string& nodeId) = 0;
};

using SettingsTreePtr = std::shared_ptr<ISettingsTree>;

} // namespace commonHead::viewModels::model
