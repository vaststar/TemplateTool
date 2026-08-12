#include "SettingsViewModel.h"
#include "LoggerDefine.h"

#include "SettingsModel.h"

#include <functional>

#include <ResourceString.h>

#include <commonhead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonhead/ResourceLoader/IResourceLoader.h>
#include <commonhead/viewmodels/SettingsViewModel/SettingsViewModelCreator.h>

namespace commonHead::viewModels {

namespace impl {

std::shared_ptr<ISettingsViewModel> createSettingsViewModel(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<SettingsViewModel>(commonHeadFramework);
}

} // namespace impl

SettingsViewModel::SettingsViewModel(commonHead::ICommonHeadFrameworkWptr framework)
    : ISettingsViewModel(framework)
{
    SETTINGS_VIEW_MODEL_LOG_DEBUG("create SettingsViewModel");
}

std::string SettingsViewModel::getViewModelName() const
{
    return "SettingsViewModel";
}

void SettingsViewModel::init()
{
    SETTINGS_VIEW_MODEL_LOG_DEBUG("SettingsViewModel::init");
    buildSettingsTree();
    m_ready = true;
    fireNotification(&ISettingsViewModelCallback::onSettingsTreeReady);
}

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
    SETTINGS_VIEW_MODEL_LOG_DEBUG("SettingsViewModel::selectNode: " << nodeId);
}

void SettingsViewModel::reloadTree()
{
    SETTINGS_VIEW_MODEL_LOG_DEBUG("SettingsViewModel::reloadTree");

    if (!m_settingsTree)
    {
        buildSettingsTree();
        m_ready = true;
        fireNotification(&ISettingsViewModelCallback::onSettingsTreeReady);
        return;
    }

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

    for (const auto& [nodeId, token] : nodeTokenMap)
    {
        auto node = m_settingsTree->findNodeById(nodeId);
        if (node)
        {
            auto data = node->getNodeData();
            data.title = resourceLoader->getLocalizedString(token);
            node->setNodeData(data);
        }
    }
}

std::vector<model::SettingsNodeData> SettingsViewModel::snapshotAllNodes() const
{
    std::vector<model::SettingsNodeData> out;
    if (!m_settingsTree)
    {
        return out;
    }
    auto root = m_settingsTree->getRoot();
    if (!root)
    {
        return out;
    }

    std::function<void(const model::SettingsTreeNodePtr&)> dfs;
    dfs = [&](const model::SettingsTreeNodePtr& node)
    {
        if (!node)
        {
            return;
        }
        const std::size_t count = node->getChildCount();
        for (std::size_t index = 0; index < count; ++index)
        {
            auto child = node->getChild(index);
            if (!child)
            {
                continue;
            }
            out.push_back(child->getNodeData());
            dfs(child);
        }
    };
    dfs(root);
    return out;
}

void SettingsViewModel::buildSettingsTree()
{
    auto tree = std::make_shared<model::SettingsTree>();
    auto resourceLoader = getCommonHeadFramework().lock()->getResourceLoader();

    tree->addNode("", {
        "general",
        "",
        resourceLoader->getLocalizedString(commonHead::model::LocalizedString::SettingsCategoryGeneral),
        commonHead::model::AssetImageToken::Nav_Settings,
        model::SettingsPanelType::None
    });

    tree->addNode("general", {
        "general.appearance",
        "general",
        resourceLoader->getLocalizedString(commonHead::model::LocalizedString::SettingsAppearance),
        commonHead::model::AssetImageToken::None,
        model::SettingsPanelType::Appearance
    });

    tree->addNode("general", {
        "general.language",
        "general",
        resourceLoader->getLocalizedString(commonHead::model::LocalizedString::SettingsLanguage),
        commonHead::model::AssetImageToken::None,
        model::SettingsPanelType::Language
    });

    m_settingsTree = tree;

    SETTINGS_VIEW_MODEL_LOG_DEBUG("SettingsViewModel::buildSettingsTree completed");
}

} // namespace commonHead::viewModels
