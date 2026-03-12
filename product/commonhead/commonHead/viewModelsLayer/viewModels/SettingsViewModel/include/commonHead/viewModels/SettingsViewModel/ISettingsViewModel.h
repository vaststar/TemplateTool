#pragma once

#include <memory>
#include <string>

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>
#include <commonHead/commonHeadUtils/VMNotificationHelper/IVMNotificationHelper.h>
#include <commonHead/viewModels/IViewModel/IViewModel.h>
#include <commonHead/viewModels/SettingsViewModel/ISettingsModel.h>

namespace commonHead::viewModels {

/**
 * @brief Settings ViewModel callback interface
 */
class COMMONHEAD_EXPORT ISettingsViewModelCallback
{
public:
    ISettingsViewModelCallback() = default;
    ISettingsViewModelCallback(const ISettingsViewModelCallback&) = delete;
    ISettingsViewModelCallback(ISettingsViewModelCallback&&) = delete;
    ISettingsViewModelCallback& operator=(const ISettingsViewModelCallback&) = delete;
    ISettingsViewModelCallback& operator=(ISettingsViewModelCallback&&) = delete;
    virtual ~ISettingsViewModelCallback() = default;

public:
    /**
     * @brief Called when the settings tree is rebuilt (e.g. language change)
     * @param tree New settings tree
     */
    virtual void onSettingsTreeChanged(const model::SettingsTreePtr& tree) = 0;

    /**
     * @brief Called when the current selected node changed
     * @param nodeId Node ID of the newly selected node
     * @param panelType Panel type of the newly selected node
     */
    virtual void onCurrentSettingsNodeChanged(const std::string& nodeId, model::SettingsPanelType panelType) = 0;
};

class COMMONHEAD_EXPORT ISettingsViewModel 
    : public IViewModel
    , public virtual commonHead::utilities::IVMNotificationHelper<ISettingsViewModelCallback>
{
public:
    using IViewModel::IViewModel;
    ISettingsViewModel(const ISettingsViewModel&) = delete;
    ISettingsViewModel(ISettingsViewModel&&) = delete;
    ISettingsViewModel& operator=(const ISettingsViewModel&) = delete;
    ISettingsViewModel& operator=(ISettingsViewModel&&) = delete;
    virtual ~ISettingsViewModel() = default;

public:
    virtual std::string getViewModelName() const = 0;

    virtual model::SettingsTreePtr getSettingsTree() const = 0;

    /**
     * @brief Get current selected node ID
     */
    virtual std::string getCurrentNodeId() const = 0;

    /**
     * @brief Get current selected panel type
     */
    virtual model::SettingsPanelType getCurrentPanelType() const = 0;

    /**
     * @brief Select a node by ID
     * @param nodeId Node ID to select
     */
    virtual void selectNode(const std::string& nodeId) = 0;

    /**
     * @brief 重新构建设置树（语言切换后刷新本地化字符串）
     */
    virtual void reloadTree() = 0;

public:
    static std::shared_ptr<ISettingsViewModel> createInstance(
        commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
};

} // namespace commonHead::viewModels
