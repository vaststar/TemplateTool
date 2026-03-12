#pragma once

#include <commonHead/commonHeadUtils/VMNotificationHelper/VMNotificationHelper.h>
#include <commonHead/viewModels/ToolsViewModel/IToolsViewModel.h>

namespace commonHead::viewModels {

class ToolsViewModel : public virtual IToolsViewModel,
                       public virtual commonHead::utilities::VMNotificationHelper<IToolsViewModelCallback>
{
public:
    explicit ToolsViewModel(commonHead::ICommonHeadFrameworkWptr framework);
    ~ToolsViewModel() override = default;

    std::string getViewModelName() const override;

    //========================================
    // 工具树导航
    //========================================
    model::ToolsTreePtr getToolsTree() const override;
    std::string getCurrentNodeId() const override;
    model::ToolPanelType getCurrentPanelType() const override;
    void selectNode(const std::string& nodeId) override;
    void reloadTree() override;

    //========================================
    // Base64 工具
    //========================================
    Base64Result base64Encode(const std::string& input, bool urlSafe = false) override;
    Base64Result base64Decode(const std::string& input) override;

    //========================================
    // JSON 工具
    //========================================
    JsonFormatResult jsonFormat(const std::string& input, int indent = 2) override;
    JsonFormatResult jsonMinify(const std::string& input) override;
    JsonFormatResult jsonValidate(const std::string& input) override;

    //========================================
    // 时间戳工具
    //========================================
    TimestampResult timestampToDateTime(int64_t timestamp, bool isMilliseconds = true) override;
    TimestampResult dateTimeToTimestamp(const std::string& dateTimeStr, const std::string& format = "") override;
    TimestampResult getCurrentTimestamp() override;

    //========================================
    // UUID 工具
    //========================================
    std::string generateUuid() override;
    bool isValidUuid(const std::string& uuid) override;

protected:
    void init() override;

private:
    void buildToolsTree();
    std::string findFirstToolNodeId() const;

private:
    model::ToolsTreePtr m_toolsTree;
    std::string m_currentNodeId;
    model::ToolPanelType m_currentPanelType = model::ToolPanelType::None;
};

} // namespace commonHead::viewModels
