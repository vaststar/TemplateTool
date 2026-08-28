#include "ToolsViewModel.h"
#include "LoggerDefine.h"

#include <functional>
#include <utility>

#include <commonhead/ResourceStringLoader/ResourceString.h>

#include <commonhead/ResourceLoader/IResourceLoader.h>
#include <commonhead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonhead/viewmodels/ToolsViewModel/ToolsViewModelCreator.h>

#include "ToolsModel.h"

namespace commonHead::viewModels {

std::shared_ptr<IToolsViewModel> impl::createToolsViewModel(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<ToolsViewModel>(commonHeadFramework);
}

// ==================== Constructor/Destructor ====================

ToolsViewModel::ToolsViewModel(commonHead::ICommonHeadFrameworkWptr framework)
    : IToolsViewModel(framework)
{
    TOOLS_VIEW_MODEL_LOG_DEBUG("ToolsViewModel constructed, address: " << this);
}

ToolsViewModel::~ToolsViewModel()
{
    TOOLS_VIEW_MODEL_LOG_DEBUG("ToolsViewModel destroying, address: " << this);
}

// ==================== IViewModel ====================

std::string ToolsViewModel::getViewModelName() const
{
    return "ToolsViewModel";
}

void ToolsViewModel::init()
{
    if (!buildToolsTree())
    {
        TOOLS_VIEW_MODEL_LOG_ERROR("ToolsViewModel::init: failed to build tools tree");
        return;
    }

    m_ready = true;
    fireNotification(&IToolsViewModelCallback::onToolsTreeReady);
}

// ==================== Public methods ====================

model::ToolsTreePtr ToolsViewModel::getToolsTree() const
{
    return m_toolsTree;
}

bool ToolsViewModel::isToolsTreeReady() const
{
    return m_ready;
}

void ToolsViewModel::selectNode(const std::string& nodeId)
{
    // VM owns no selection state; controller is the source of truth.
    // Keep this hook so future metrics/telemetry can be added in one place.
    TOOLS_VIEW_MODEL_LOG_DEBUG("ToolsViewModel::selectNode: " << nodeId);
}

void ToolsViewModel::reloadTree()
{
    TOOLS_VIEW_MODEL_LOG_DEBUG("ToolsViewModel::reloadTree");

    if (!m_toolsTree)
    {
        if (!buildToolsTree())
        {
            TOOLS_VIEW_MODEL_LOG_ERROR("ToolsViewModel::reloadTree: failed to build tools tree");
            return;
        }

        m_ready = true;
        fireNotification(&IToolsViewModelCallback::onToolsTreeReady);
        return;
    }

    // In-place refresh of all node titles/properties; ids and parent links stay
    // stable, so the UI can patch its mirror without any structural change.
    if (!refreshTreeNodeData())
    {
        TOOLS_VIEW_MODEL_LOG_ERROR("ToolsViewModel::reloadTree: failed to refresh tools tree");
        return;
    }

    fireNotification(&IToolsViewModelCallback::onToolsNodesUpdated,
                     snapshotAllNodes());
}

bool ToolsViewModel::refreshTreeNodeData()
{
    auto resourceLoader = lockResourceLoader();
    if (!resourceLoader)
    {
        return false;
    }

    static const std::vector<std::pair<std::string, commonHead::model::LocalizedString>> nodeTokenMap = {
        { "text",               commonHead::model::LocalizedString::ToolsCategoryText },
        { "text.base64",        commonHead::model::LocalizedString::ToolsBase64 },
        { "text.json",          commonHead::model::LocalizedString::ToolsJson },
        { "time",               commonHead::model::LocalizedString::ToolsCategoryTime },
        { "time.timestamp",     commonHead::model::LocalizedString::ToolsTimestamp },
        { "generator",          commonHead::model::LocalizedString::ToolsCategoryGenerator },
        { "generator.uuid",     commonHead::model::LocalizedString::ToolsUuid },
        { "network",            commonHead::model::LocalizedString::ToolsCategoryNetwork },
        { "network.proxy",      commonHead::model::LocalizedString::ToolsNetworkProxy },
        { "media",              commonHead::model::LocalizedString::ToolsCategoryMedia },
        { "media.screenshot",   commonHead::model::LocalizedString::ToolsScreenshot },
        { "media.recording",    commonHead::model::LocalizedString::ToolsScreenRecording },
    };

    for (const auto& [nodeId, token] : nodeTokenMap) {
        auto node = m_toolsTree->findNodeById(nodeId);
        if (node) {
            auto data = node->getNodeData();
            data.title = resourceLoader->getLocalizedString(token);
            node->setNodeData(data);
        }
    }

    return true;
}

std::vector<model::ToolNodeData> ToolsViewModel::snapshotAllNodes() const
{
    std::vector<model::ToolNodeData> out;
    if (!m_toolsTree) {
        return out;
    }
    auto root = m_toolsTree->getRoot();
    if (!root) {
        return out;
    }

    std::function<void(const model::ToolsTreeNodePtr&)> dfs;
    dfs = [&](const model::ToolsTreeNodePtr& node) {
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

std::shared_ptr<commonHead::IResourceLoader> ToolsViewModel::lockResourceLoader() const
{
    auto framework = getCommonHeadFramework().lock();
    if (!framework)
    {
        TOOLS_VIEW_MODEL_LOG_ERROR(
            "ToolsViewModel::lockResourceLoader: CommonHeadFramework is not available");
        return {};
    }

    auto resourceLoader = framework->getResourceLoader();
    if (!resourceLoader)
    {
        TOOLS_VIEW_MODEL_LOG_ERROR(
            "ToolsViewModel::lockResourceLoader: ResourceLoader is not available");
        return {};
    }

    return resourceLoader;
}

bool ToolsViewModel::buildToolsTree()
{
    auto resourceLoader = lockResourceLoader();
    if (!resourceLoader)
    {
        return false;
    }

    auto tree = std::make_shared<model::ToolsTree>();

    // Note: ToolsTree::addNode() stamps the parentId argument into the node data;
    // we still pass an explicit empty string here so the initializer matches the
    // 5-field ToolNodeData aggregate (nodeId, parentId, title, icon, panelType).

    // Text Processing category
    tree->addNode("", {
        "text",
        "",
        resourceLoader->getLocalizedString(commonHead::model::LocalizedString::ToolsCategoryText),
        "",
        model::ToolPanelType::None
    });

    // Text Processing > Base64 Encode/Decode
    tree->addNode("text", {
        "text.base64",
        "",
        resourceLoader->getLocalizedString(commonHead::model::LocalizedString::ToolsBase64),
        "",
        model::ToolPanelType::Base64
    });

    // Text Processing > JSON Formatter
    tree->addNode("text", {
        "text.json",
        "",
        resourceLoader->getLocalizedString(commonHead::model::LocalizedString::ToolsJson),
        "",
        model::ToolPanelType::Json
    });

    // Date & Time category
    tree->addNode("", {
        "time",
        "",
        resourceLoader->getLocalizedString(commonHead::model::LocalizedString::ToolsCategoryTime),
        "",
        model::ToolPanelType::None
    });

    // Date & Time > Timestamp Converter
    tree->addNode("time", {
        "time.timestamp",
        "",
        resourceLoader->getLocalizedString(commonHead::model::LocalizedString::ToolsTimestamp),
        "",
        model::ToolPanelType::Timestamp
    });

    // Generators category
    tree->addNode("", {
        "generator",
        "",
        resourceLoader->getLocalizedString(commonHead::model::LocalizedString::ToolsCategoryGenerator),
        "",
        model::ToolPanelType::None
    });

    // Generators > UUID Generator
    tree->addNode("generator", {
        "generator.uuid",
        "",
        resourceLoader->getLocalizedString(commonHead::model::LocalizedString::ToolsUuid),
        "",
        model::ToolPanelType::Uuid
    });

    // Network category
    tree->addNode("", {
        "network",
        "",
        resourceLoader->getLocalizedString(commonHead::model::LocalizedString::ToolsCategoryNetwork),
        "",
        model::ToolPanelType::None
    });

    // Network > Network Proxy
    tree->addNode("network", {
        "network.proxy",
        "",
        resourceLoader->getLocalizedString(commonHead::model::LocalizedString::ToolsNetworkProxy),
        "",
        model::ToolPanelType::NetworkProxy
    });

    // Media category
    tree->addNode("", {
        "media",
        "",
        resourceLoader->getLocalizedString(commonHead::model::LocalizedString::ToolsCategoryMedia),
        "",
        model::ToolPanelType::None
    });

    // Media > Screenshot
    tree->addNode("media", {
        "media.screenshot",
        "",
        resourceLoader->getLocalizedString(commonHead::model::LocalizedString::ToolsScreenshot),
        "",
        model::ToolPanelType::Screenshot
    });

    // Media > Screen Recording
    tree->addNode("media", {
        "media.recording",
        "",
        resourceLoader->getLocalizedString(commonHead::model::LocalizedString::ToolsScreenRecording),
        "",
        model::ToolPanelType::ScreenRecording
    });

    m_toolsTree = std::move(tree);

    TOOLS_VIEW_MODEL_LOG_DEBUG("ToolsViewModel::buildToolsTree completed");

    return true;
}

} // namespace commonHead::viewModels
