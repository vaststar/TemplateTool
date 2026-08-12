#pragma once

#include <memory>

#include <commonhead/utilities/VMNotificationHelper/VMNotificationHelper.h>
#include <commonhead/viewmodels/MainWindowViewModel/IMainWindowViewModel.h>
#include <ucf/services/InvocationService/IInvocationServiceCallback.h>

namespace commonHead::viewModels{
class MainWindowViewModel: public virtual IMainWindowViewModel,
                           public virtual commonHead::utilities::VMNotificationHelper<IMainWindowViewModelCallback>,
                           public ucf::service::IInvocationServiceCallback,
                           public std::enable_shared_from_this<MainWindowViewModel>
{
public:
    explicit MainWindowViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
    MainWindowViewModel(const MainWindowViewModel&) = delete;
    MainWindowViewModel(MainWindowViewModel&&) = delete;
    MainWindowViewModel& operator=(const MainWindowViewModel&) = delete;
    MainWindowViewModel& operator=(MainWindowViewModel&&) = delete;
    ~MainWindowViewModel() override = default;
public:
    std::string getViewModelName() const override;

    void onCommandMessageReceived(const std::string& message) override;

    void packApplicationLogs() override;

protected:
    void init() override;
};
}
