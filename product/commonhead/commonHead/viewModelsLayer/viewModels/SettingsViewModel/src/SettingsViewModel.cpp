#include "SettingsViewModel.h"
#include "SettingsModel.h"

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>

namespace commonHead::viewModels {

// ==================== Factory method ====================

std::shared_ptr<ISettingsViewModel> ISettingsViewModel::createInstance(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<SettingsViewModel>(commonHeadFramework);
}

// ==================== Constructor/Destructor ====================

SettingsViewModel::SettingsViewModel(commonHead::ICommonHeadFrameworkWptr framework)
    : ISettingsViewModel(framework)
{
    COMMONHEAD_LOG_DEBUG("create SettingsViewModel");
}

// ==================== IViewModel ====================

std::string SettingsViewModel::getViewModelName() const
{
    return "SettingsViewModel";
}

void SettingsViewModel::init()
{
    COMMONHEAD_LOG_DEBUG("SettingsViewModel::init");
    buildSettingsTree();
}

// ==================== Public methods ====================

model::SettingsTreePtr SettingsViewModel::getSettingsTree() const
{
    return m_settingsTree;
}

// ==================== Private methods ====================

void SettingsViewModel::buildSettingsTree()
{
    auto tree = std::make_shared<model::SettingsTree>();

    // General category (empty parentId means add to virtual root)
    tree->addNode("", {
        "general",
        "General",
        commonHead::model::AssetImageToken::Nav_Settings,
        model::SettingsPanelType::None
    });

    // General > Appearance
    tree->addNode("general", {
        "general.appearance",
        "Appearance",
        commonHead::model::AssetImageToken::None,
        model::SettingsPanelType::Appearance
    });

    // General > Language
    tree->addNode("general", {
        "general.language",
        "Language",
        commonHead::model::AssetImageToken::None,
        model::SettingsPanelType::Language
    });

    m_settingsTree = tree;
    
    COMMONHEAD_LOG_DEBUG("SettingsViewModel::buildSettingsTree completed");
}

} // namespace commonHead::viewModels
