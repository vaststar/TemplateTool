#pragma once

#include <memory>
#include <string>

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>
#include <commonHead/commonHeadUtils/VMNotificationHelper/IVMNotificationHelper.h>
#include <commonHead/viewModels/IViewModel/IViewModel.h>
#include <commonHead/viewModels/ToolsViewModel/IToolsModel.h>

namespace commonHead::viewModels {

/**
 * @brief Tools ViewModel callback interface
 */
class COMMONHEAD_EXPORT IToolsViewModelCallback
{
public:
    IToolsViewModelCallback() = default;
    IToolsViewModelCallback(const IToolsViewModelCallback&) = delete;
    IToolsViewModelCallback(IToolsViewModelCallback&&) = delete;
    IToolsViewModelCallback& operator=(const IToolsViewModelCallback&) = delete;
    IToolsViewModelCallback& operator=(IToolsViewModelCallback&&) = delete;
    virtual ~IToolsViewModelCallback() = default;

public:
    /**
     * @brief L4: Called when the entire tools tree is replaced (e.g. init, profile switch)
     * @param tree New tools tree
     */
    virtual void onToolsTreeChanged(const model::ToolsTreePtr& tree) = 0;

    /**
     * @brief L3: Called when a node is inserted or removed (same tree object)
     * @param change Describes the structural change (insert/remove, parent, index)
     */
    virtual void onToolsTreeStructureChanged(const model::ToolsTreeNodeChange& change) = 0;

    /**
     * @brief L2: Called when all node properties are refreshed in-place (e.g. language change)
     */
    virtual void onToolsTreeItemsUpdated() = 0;

    /**
     * @brief L1: Called when a single node's properties changed in-place
     * @param nodeId The node whose data was updated
     */
    virtual void onToolsTreeItemUpdated(const std::string& nodeId) = 0;

    /**
     * @brief Called when the current selected node changed
     * @param nodeId Node ID of the newly selected node
     * @param panelType Panel type of the newly selected node
     */
    virtual void onCurrentToolNodeChanged(const std::string& nodeId, model::ToolPanelType panelType) = 0;
};

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
class COMMONHEAD_EXPORT IToolsViewModel 
    : public IViewModel
    , public virtual commonHead::utilities::IVMNotificationHelper<IToolsViewModelCallback>
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

    /**
     * @brief Get current selected node ID
     */
    virtual std::string getCurrentNodeId() const = 0;

    /**
     * @brief Get current selected panel type
     */
    virtual model::ToolPanelType getCurrentPanelType() const = 0;

    /**
     * @brief Select a node by ID
     * @param nodeId Node ID to select
     */
    virtual void selectNode(const std::string& nodeId) = 0;

    /**
     * @brief 重新构建工具树（语言切换后刷新本地化字符串）
     */
    virtual void reloadTree() = 0;

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
