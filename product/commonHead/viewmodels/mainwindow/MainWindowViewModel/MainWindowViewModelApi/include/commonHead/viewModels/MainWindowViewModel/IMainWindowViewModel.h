#pragma once

#include <string>

#include <commonHead/commonHeadUtils/VMNotificationHelper/IVMNotificationHelper.h>
#include <commonHead/viewModels/IViewModel/IViewModel.h>

namespace commonHead::viewModels{
class IMainWindowViewModelCallback
{
public:
    IMainWindowViewModelCallback() = default;
    IMainWindowViewModelCallback(const IMainWindowViewModelCallback&) = delete;
    IMainWindowViewModelCallback(IMainWindowViewModelCallback&&) = delete;
    IMainWindowViewModelCallback& operator=(const IMainWindowViewModelCallback&) = delete;
    IMainWindowViewModelCallback& operator=(IMainWindowViewModelCallback&&) = delete;
    virtual ~IMainWindowViewModelCallback() = default;
public:
    virtual void onActivateMainWindow() = 0;
    virtual void onLogsPackComplete(bool success, const std::string& archivePath) = 0;
};

class IMainWindowViewModel: public IViewModel, public virtual commonHead::utilities::IVMNotificationHelper<IMainWindowViewModelCallback>
{
public:
    using IViewModel::IViewModel;
    IMainWindowViewModel(const IMainWindowViewModel&) = delete;
    IMainWindowViewModel(IMainWindowViewModel&&) = delete;
    IMainWindowViewModel& operator=(const IMainWindowViewModel&) = delete;
    IMainWindowViewModel& operator=(IMainWindowViewModel&&) = delete;
    virtual ~IMainWindowViewModel() = default;
public:
    /**
     * @brief Pack application logs into a ZIP archive
     * The result will be notified via onLogsPackComplete callback
     */
    virtual void packApplicationLogs() = 0;
};
}
