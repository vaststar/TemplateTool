#include "ToolsViewModel.h"
#include "ToolsModel.h"

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>
#include <ucf/Utilities/Base64Utils/Base64Utils.h>
#include <ucf/Utilities/JsonUtils/JsonValue.h>
#include <ucf/Utilities/TimeUtils/TimeUtils.h>
#include <ucf/Utilities/UUIDUtils/UUIDUtils.h>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

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
}

// ==================== Public methods ====================

model::ToolsTreePtr ToolsViewModel::getToolsTree() const
{
    return m_toolsTree;
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
        result.data = "JSON 格式有效";
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
            result.errorMessage = "无效的时间戳";
            return result;
        }
        
        std::ostringstream oss;
        oss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
        
        result.success = true;
        result.timestamp = timestamp;
        result.dateTimeStr = oss.str();
        result.timezone = ucf::utilities::TimeUtils::getLocalTimeZone();
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
            result.errorMessage = "日期时间格式解析失败";
            return result;
        }
        
        time_t seconds = std::mktime(&tm);
        if (seconds == -1)
        {
            result.success = false;
            result.errorMessage = "无效的日期时间";
            return result;
        }
        
        result.success = true;
        result.timestamp = static_cast<int64_t>(seconds) * 1000;  // Return milliseconds
        result.dateTimeStr = dateTimeStr;
        result.timezone = ucf::utilities::TimeUtils::getLocalTimeZone();
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
    result.timestamp = ucf::utilities::TimeUtils::getUTCNowInMilliseconds();
    result.dateTimeStr = ucf::utilities::TimeUtils::getUTCCurrentTime();
    result.timezone = ucf::utilities::TimeUtils::getLocalTimeZone();
    
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

    // 文本处理分类 (empty parentId means add to virtual root)
    tree->addNode("", {
        "text",
        "文本处理",
        "",
        model::ToolPanelType::None
    });

    // 文本处理 > Base64 编解码
    tree->addNode("text", {
        "text.base64",
        "Base64 编解码",
        "",
        model::ToolPanelType::Base64
    });

    // 文本处理 > JSON 格式化
    tree->addNode("text", {
        "text.json",
        "JSON 格式化",
        "",
        model::ToolPanelType::Json
    });

    // 时间日期分类
    tree->addNode("", {
        "time",
        "时间日期",
        "",
        model::ToolPanelType::None
    });

    // 时间日期 > 时间戳转换
    tree->addNode("time", {
        "time.timestamp",
        "时间戳转换",
        "",
        model::ToolPanelType::Timestamp
    });

    // 生成器分类
    tree->addNode("", {
        "generator",
        "生成器",
        "",
        model::ToolPanelType::None
    });

    // 生成器 > UUID 生成
    tree->addNode("generator", {
        "generator.uuid",
        "UUID 生成",
        "",
        model::ToolPanelType::Uuid
    });

    m_toolsTree = tree;
    
    COMMONHEAD_LOG_DEBUG("ToolsViewModel::buildToolsTree completed");
}

} // namespace commonHead::viewModels
