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
    m_ready = true;
    fireNotification(&ISettingsViewModelCallback::onSettingsTreeReady);
}

// ==================== Public methods ====================

model::SettingsTreePtr SettingsViewModel::getSettingsTree() const
{
    return m_settingsTree;
}

bool SettingsViewModel::isSettingsTreeReady() const
{
    return m_ready;
}

void SettingsViewModel::selectNode(const std::string& nodeId)
{
    // VM owns no selection state; controller is the source of truth.
    // Keep this hook so future metrics/telemetry can be added in one place.
    COMMONHEAD_LOG_DEBUG("SettingsViewModel::selectNode: " << nodeId);
}

void SettingsViewModel::reloadTree()
{
    COMMONHEAD_LOG_DEBUG("SettingsViewModel::reloadTree");

    if (!m_settingsTree) {
        // First-time build path: act like init().
        buildSettingsTree();
        m_ready = true;
        fireNotification(&ISettingsViewModelCallback::onSettingsTreeReady);
        return;
    }

    // In-place refresh of all node titles/properties; ids and parent links stay
    // stable, so the UI can patch its mirror without any structural change.
    refreshTreeNodeData();
    fireNotification(&ISettingsViewModelCallback::onSettingsNodesUpdated,
                     snapshotAllNodes());
}

void SettingsViewModel::refreshTreeNodeData()
{
    auto resourceLoader = getCommonHeadFramework().lock()->getResourceLoader();

    static const std::vector<std::pair<std::string, commonHead::model::LocalizedString>> nodeTokenMap = {
        { "general",              commonHead::model::LocalizedString::SettingsCategoryGeneral },
        { "general.appearance",   commonHead::model::LocalizedString::SettingsAppearance },
        { "general.language",     commonHead::model::LocalizedString::SettingsLanguage },
    };

    for (const auto& [nodeId, token] : nodeTokenMap) {
        auto node = m_settingsTree->findNodeById(nodeId);
        if (node) {
            auto data = node->getNodeData();
            data.title = resourceLoader->getLocalizedString(token);
            node->setNodeData(data);
        }
    }
}

std::vector<model::SettingsNodeData> SettingsViewModel::snapshotAllNodes() const
{
    std::vector<model::SettingsNodeData> out;
    if (!m_settingsTree) {
        return out;
    }
    auto root = m_settingsTree->getRoot();
    if (!root) {
        return out;
    }

    std::function<void(const model::SettingsTreeNodePtr&)> dfs;
    dfs = [&](const model::SettingsTreeNodePtr& node) {
        if (!node) return;
        const std::size_t n = node->getChildCount();
        for (std::size_t i = 0; i < n; ++i) {
            auto child = node->getChild(i);
            if (!child) continue;
            out.push_back(child->getNodeData());
            dfs(child);
        }
    };
    dfs(root);
    return out;
}

// ==================== Private methods ====================

void SettingsViewModel::buildSettingsTree()
{
    auto tree = std::make_shared<model::SettingsTree>();
    auto resourceLoader = getCommonHeadFramework().lock()->getResourceLoader();

    // General category (empty parentId means add to virtual root)
    tree->addNode("", {
        "general",
        "",
        resourceLoader->getLocalizedString(commonHead::model::LocalizedString::SettingsCategoryGeneral),
        commonHead::model::AssetImageToken::Nav_Settings,
        model::SettingsPanelType::None
    });

    // General > Appearance
    tree->addNode("general", {
        "general.appearance",
        "general",
        resourceLoader->getLocalizedString(commonHead::model::LocalizedString::SettingsAppearance),
        commonHead::model::AssetImageToken::None,
        model::SettingsPanelType::Appearance
    });

    // General > Language
    tree->addNode("general", {
        "general.language",
        "general",
        resourceLoader->getLocalizedString(commonHead::model::LocalizedString::SettingsLanguage),
        commonHead::model::AssetImageToken::None,
        model::SettingsPanelType::Language
    });

    m_settingsTree = tree;

    COMMONHEAD_LOG_DEBUG("SettingsViewModel::buildSettingsTree completed");
}

} // namespace commonHead::viewModels
