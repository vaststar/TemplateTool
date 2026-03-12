#include "SettingsViewModel.h"
#include "SettingsModel.h"

#include <functional>

#include <ResourceString.h>

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>
#include <commonHead/ResourceLoader/IResourceLoader.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>

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

    // Select first settings node by default
    std::string firstId = findFirstSettingsNodeId();
    if (!firstId.empty()) {
        selectNode(firstId);
    }
}

// ==================== Public methods ====================

model::SettingsTreePtr SettingsViewModel::getSettingsTree() const
{
    return m_settingsTree;
}

std::string SettingsViewModel::getCurrentNodeId() const
{
    return m_currentNodeId;
}

model::SettingsPanelType SettingsViewModel::getCurrentPanelType() const
{
    return m_currentPanelType;
}

void SettingsViewModel::selectNode(const std::string& nodeId)
{
    if (!m_settingsTree)
        return;

    auto node = m_settingsTree->findNodeById(nodeId);
    if (!node)
        return;

    auto nodeData = node->getNodeData();

    // Only update if the node has a panel (not a category)
    if (nodeData.panelType != model::SettingsPanelType::None) {
        m_currentNodeId = nodeId;
        m_currentPanelType = nodeData.panelType;
        fireNotification(&ISettingsViewModelCallback::onCurrentSettingsNodeChanged, m_currentNodeId, m_currentPanelType);
    }
}

void SettingsViewModel::reloadTree()
{
    COMMONHEAD_LOG_DEBUG("SettingsViewModel::reloadTree");

    // Save current selection
    std::string savedNodeId = m_currentNodeId;

    // Rebuild tree
    buildSettingsTree();
    fireNotification(&ISettingsViewModelCallback::onSettingsTreeChanged, m_settingsTree);

    // Restore selection if the node still exists, otherwise select first
    if (!savedNodeId.empty() && m_settingsTree->findNodeById(savedNodeId)) {
        selectNode(savedNodeId);
    } else {
        std::string firstId = findFirstSettingsNodeId();
        if (!firstId.empty()) {
            selectNode(firstId);
        }
    }
}

// ==================== Private methods ====================

void SettingsViewModel::buildSettingsTree()
{
    auto tree = std::make_shared<model::SettingsTree>();
    auto resourceLoader = getCommonHeadFramework().lock()->getResourceLoader();

    // General category (empty parentId means add to virtual root)
    tree->addNode("", {
        "general",
        resourceLoader->getLocalizedString(commonHead::model::LocalizedString::SettingsCategoryGeneral),
        commonHead::model::AssetImageToken::Nav_Settings,
        model::SettingsPanelType::None
    });

    // General > Appearance
    tree->addNode("general", {
        "general.appearance",
        resourceLoader->getLocalizedString(commonHead::model::LocalizedString::SettingsAppearance),
        commonHead::model::AssetImageToken::None,
        model::SettingsPanelType::Appearance
    });

    // General > Language
    tree->addNode("general", {
        "general.language",
        resourceLoader->getLocalizedString(commonHead::model::LocalizedString::SettingsLanguage),
        commonHead::model::AssetImageToken::None,
        model::SettingsPanelType::Language
    });

    m_settingsTree = tree;
    
    COMMONHEAD_LOG_DEBUG("SettingsViewModel::buildSettingsTree completed");
}

std::string SettingsViewModel::findFirstSettingsNodeId() const
{
    if (!m_settingsTree)
        return "";

    auto root = m_settingsTree->getRoot();
    if (!root || root->getChildCount() == 0)
        return "";

    // DFS to find first node with a panel (not a category)
    std::function<std::string(const model::SettingsTreeNodePtr&)> findFirst;
    findFirst = [&findFirst](const model::SettingsTreeNodePtr& node) -> std::string {
        if (!node)
            return "";

        auto data = node->getNodeData();
        if (data.panelType != model::SettingsPanelType::None) {
            return data.nodeId;
        }

        for (std::size_t i = 0; i < node->getChildCount(); ++i) {
            auto child = node->getChild(i);
            auto result = findFirst(child);
            if (!result.empty())
                return result;
        }
        return "";
    };

    return findFirst(root);
}

} // namespace commonHead::viewModels
