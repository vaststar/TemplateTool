#include "ToolsViewModel.h"

#include <chrono>
#include <ctime>
#include <functional>
#include <iomanip>
#include <sstream>

#include <ucf/Utilities/Base64Utils/Base64Utils.h>
#include <ucf/Utilities/JsonUtils/JsonValue.h>
#include <ucf/Utilities/TimeUtils/TimeUtils.h>
#include <ucf/Utilities/UUIDUtils/UUIDUtils.h>

#include <ResourceString.h>

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>
#include <commonHead/ResourceLoader/IResourceLoader.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>

#include "ToolsModel.h"

namespace commonHead::viewModels {

// ==================== Factory method ====================

std::shared_ptr<IToolsViewModel> IToolsViewModel::createInstance(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<ToolsViewModel>(commonHeadFramework);
}

// ==================== Constructor/Destructor ====================

ToolsViewModel::ToolsViewModel(commonHead::ICommonHeadFrameworkWptr framework)
    : IToolsViewModel(framework)
{
    COMMONHEAD_LOG_DEBUG("create ToolsViewModel");
}

// ==================== IViewModel ====================

std::string ToolsViewModel::getViewModelName() const
{
    return "ToolsViewModel";
}

void ToolsViewModel::init()
{
    COMMONHEAD_LOG_DEBUG("ToolsViewModel::init");
    buildToolsTree();
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
    COMMONHEAD_LOG_DEBUG("ToolsViewModel::selectNode: " << nodeId);
}

void ToolsViewModel::reloadTree()
{
    COMMONHEAD_LOG_DEBUG("ToolsViewModel::reloadTree");

    if (!m_toolsTree) {
        // First-time build path: act like init().
        buildToolsTree();
        m_ready = true;
        fireNotification(&IToolsViewModelCallback::onToolsTreeReady);
        return;
    }

    // In-place refresh of all node titles/properties; ids and parent links stay
    // stable, so the UI can patch its mirror without any structural change.
    refreshTreeNodeData();
    fireNotification(&IToolsViewModelCallback::onToolsNodesUpdated,
                     snapshotAllNodes());
}

void ToolsViewModel::refreshTreeNodeData()
{
    auto resourceLoader = getCommonHeadFramework().lock()->getResourceLoader();

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

// ==================== Base64 工具 ====================

Base64Result ToolsViewModel::base64Encode(const std::string& input, bool urlSafe)
{
    Base64Result result;

    auto variant = urlSafe ? ucf::utilities::Base64Variant::UrlSafe
                           : ucf::utilities::Base64Variant::Standard;
    auto encodeResult = ucf::utilities::Base64Utils::encode(input, variant);

    result.success = encodeResult.success;
    result.data = encodeResult.data;
    result.errorMessage = encodeResult.errorMessage;

    return result;
}

Base64Result ToolsViewModel::base64Decode(const std::string& input)
{
    Base64Result result;

    auto decodeResult = ucf::utilities::Base64Utils::decode(input);

    result.success = decodeResult.success;
    if (decodeResult.success)
    {
        result.data = std::string(decodeResult.data.begin(), decodeResult.data.end());
    }
    result.errorMessage = decodeResult.errorMessage;

    return result;
}

// ==================== JSON 工具 ====================

JsonFormatResult ToolsViewModel::jsonFormat(const std::string& input, int indent)
{
    JsonFormatResult result;

    auto parseResult = ucf::utilities::JsonValue::parseEx(input);
    if (!parseResult.ok())
    {
        result.success = false;
        result.errorMessage = parseResult.error;
        return result;
    }

    result.success = true;
    result.data = parseResult.value.dumpPretty(indent);
    return result;
}

JsonFormatResult ToolsViewModel::jsonMinify(const std::string& input)
{
    JsonFormatResult result;

    auto parseResult = ucf::utilities::JsonValue::parseEx(input);
    if (!parseResult.ok())
    {
        result.success = false;
        result.errorMessage = parseResult.error;
        return result;
    }

    result.success = true;
    result.data = parseResult.value.dump();
    return result;
}

JsonFormatResult ToolsViewModel::jsonValidate(const std::string& input)
{
    JsonFormatResult result;

    auto parseResult = ucf::utilities::JsonValue::parseEx(input);
    result.success = parseResult.ok();
    if (!result.success)
    {
        result.errorMessage = parseResult.error;
    }
    else
    {
        auto resourceLoader = getCommonHeadFramework().lock()->getResourceLoader();
        result.data = resourceLoader->getLocalizedString(commonHead::model::LocalizedString::ToolsJsonValid);
    }
    return result;
}

// ==================== 时间戳工具 ====================

TimestampResult ToolsViewModel::timestampToDateTime(int64_t timestamp, bool isMilliseconds)
{
    TimestampResult result;

    try
    {
        // Convert to seconds if milliseconds
        time_t seconds = isMilliseconds ? (timestamp / 1000) : timestamp;

        std::tm* tm = std::localtime(&seconds);
        if (!tm)
        {
            result.success = false;
            auto resourceLoader = getCommonHeadFramework().lock()->getResourceLoader();
            result.errorMessage = resourceLoader->getLocalizedString(commonHead::model::LocalizedString::ToolsInvalidTimestamp);
            return result;
        }

        std::ostringstream oss;
        oss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");

        result.success = true;
        result.timestamp = timestamp;
        result.dateTimeStr = oss.str();
        result.timezone = ucf::utilities::TimeUtils::getLocalTimeZoneName();
    }
    catch (const std::exception& e)
    {
        result.success = false;
        result.errorMessage = e.what();
    }

    return result;
}

TimestampResult ToolsViewModel::dateTimeToTimestamp(const std::string& dateTimeStr, const std::string& format)
{
    TimestampResult result;

    try
    {
        std::tm tm = {};
        std::istringstream iss(dateTimeStr);

        std::string fmt = format.empty() ? "%Y-%m-%d %H:%M:%S" : format;
        iss >> std::get_time(&tm, fmt.c_str());

        if (iss.fail())
        {
            result.success = false;
            auto resourceLoader = getCommonHeadFramework().lock()->getResourceLoader();
            result.errorMessage = resourceLoader->getLocalizedString(commonHead::model::LocalizedString::ToolsDateTimeParseFailed);
            return result;
        }

        time_t seconds = std::mktime(&tm);
        if (seconds == -1)
        {
            result.success = false;
            auto resourceLoader = getCommonHeadFramework().lock()->getResourceLoader();
            result.errorMessage = resourceLoader->getLocalizedString(commonHead::model::LocalizedString::ToolsInvalidDateTime);
            return result;
        }

        result.success = true;
        result.timestamp = static_cast<int64_t>(seconds) * 1000;  // Return milliseconds
        result.dateTimeStr = dateTimeStr;
        result.timezone = ucf::utilities::TimeUtils::getLocalTimeZoneName();
    }
    catch (const std::exception& e)
    {
        result.success = false;
        result.errorMessage = e.what();
    }

    return result;
}

TimestampResult ToolsViewModel::getCurrentTimestamp()
{
    TimestampResult result;

    result.success = true;
    result.timestamp = ucf::utilities::TimeUtils::getCurrentUTCMilliseconds();
    result.dateTimeStr = ucf::utilities::TimeUtils::getCurrentUTCTimeString();
    result.timezone = ucf::utilities::TimeUtils::getLocalTimeZoneName();

    return result;
}

// ==================== UUID 工具 ====================

std::string ToolsViewModel::generateUuid()
{
    return ucf::utilities::UUIDUtils::generateUUID();
}

bool ToolsViewModel::isValidUuid(const std::string& uuid)
{
    return ucf::utilities::UUIDUtils::isValidUUID(uuid);
}

// ==================== Private methods ====================

void ToolsViewModel::buildToolsTree()
{
    auto tree = std::make_shared<model::ToolsTree>();


    auto resourceLoader = getCommonHeadFramework().lock()->getResourceLoader();

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

    m_toolsTree = tree;

    COMMONHEAD_LOG_DEBUG("ToolsViewModel::buildToolsTree completed");
}

} // namespace commonHead::viewModels
