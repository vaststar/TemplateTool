#pragma once

#include <memory>
#include <string>

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>
#include <commonHead/viewModels/IViewModel/IViewModel.h>
#include <commonHead/viewModels/ToolsViewModel/IToolsModel.h>

namespace commonHead::viewModels {

/**
 * @brief Base64 操作结果
 */
struct COMMONHEAD_EXPORT Base64Result {
    bool success = false;
    std::string data;
    std::string errorMessage;
};

/**
 * @brief JSON 操作结果
 */
struct COMMONHEAD_EXPORT JsonFormatResult {
    bool success = false;
    std::string data;
    std::string errorMessage;
};

/**
 * @brief 时间戳转换结果
 */
struct COMMONHEAD_EXPORT TimestampResult {
    bool success = false;
    int64_t timestamp = 0;          // Unix 毫秒时间戳
    std::string dateTimeStr;        // 格式化后的日期时间字符串
    std::string timezone;           // 时区
    std::string errorMessage;
};

/**
 * @brief 工具 ViewModel 接口
 */
class COMMONHEAD_EXPORT IToolsViewModel : public IViewModel
{
public:
    using IViewModel::IViewModel;
    IToolsViewModel(const IToolsViewModel&) = delete;
    IToolsViewModel(IToolsViewModel&&) = delete;
    IToolsViewModel& operator=(const IToolsViewModel&) = delete;
    IToolsViewModel& operator=(IToolsViewModel&&) = delete;
    virtual ~IToolsViewModel() = default;

public:
    virtual std::string getViewModelName() const = 0;

    //========================================
    // 工具树导航
    //========================================
    virtual model::ToolsTreePtr getToolsTree() const = 0;

    //========================================
    // Base64 工具
    //========================================
    virtual Base64Result base64Encode(const std::string& input, bool urlSafe = false) = 0;
    virtual Base64Result base64Decode(const std::string& input) = 0;

    //========================================
    // JSON 工具
    //========================================
    virtual JsonFormatResult jsonFormat(const std::string& input, int indent = 2) = 0;
    virtual JsonFormatResult jsonMinify(const std::string& input) = 0;
    virtual JsonFormatResult jsonValidate(const std::string& input) = 0;

    //========================================
    // 时间戳工具
    //========================================
    virtual TimestampResult timestampToDateTime(int64_t timestamp, bool isMilliseconds = true) = 0;
    virtual TimestampResult dateTimeToTimestamp(const std::string& dateTimeStr, const std::string& format = "") = 0;
    virtual TimestampResult getCurrentTimestamp() = 0;

    //========================================
    // UUID 工具
    //========================================
    virtual std::string generateUuid() = 0;
    virtual bool isValidUuid(const std::string& uuid) = 0;

public:
    static std::shared_ptr<IToolsViewModel> createInstance(
        commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
};

} // namespace commonHead::viewModels
