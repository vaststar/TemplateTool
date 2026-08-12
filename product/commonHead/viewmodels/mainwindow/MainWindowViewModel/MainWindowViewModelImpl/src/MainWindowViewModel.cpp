#include "MainWindowViewModel.h"
#include "LoggerDefine.h"

#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/ServiceLocator/IServiceLocator.h>
#include <commonHead/ViewModelUtils/LogOperationUtils.h>
#include <commonHead/viewModels/MainWindowViewModel/MainWindowViewModelCreator.h>
#include <ucf/Services/InvocationService/IInvocationService.h>

namespace commonHead::viewModels{

namespace impl{
std::shared_ptr<IMainWindowViewModel> createMainWindowViewModel(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<MainWindowViewModel>(commonHeadFramework);
}
} // namespace impl

MainWindowViewModel::MainWindowViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
    : IMainWindowViewModel(commonHeadFramework)
{
    MAIN_WINDOW_VIEW_MODEL_LOG_DEBUG("create MainWindowViewModel");
}

std::string MainWindowViewModel::getViewModelName() const
{
    return "MainWindowViewModel";
}

void MainWindowViewModel::init()
{
    if (auto commonHeadFramework = getCommonHeadFramework().lock())
    {
        if (auto serviceLocator = commonHeadFramework->getServiceLocator())
        {
            if (auto service = serviceLocator->getInvocationService().lock())
            {
                service->registerCallback(shared_from_this());
            }
        }
    }
}
void MainWindowViewModel::onCommandMessageReceived(const std::string& message)
{
    if (message == "ActivateWindow")
    {
        fireNotification(&IMainWindowViewModelCallback::onActivateMainWindow);
    }
}

void MainWindowViewModel::packApplicationLogs()
{
    MAIN_WINDOW_VIEW_MODEL_LOG_DEBUG("packApplicationLogs called");

    // Use LogOperationUtils to pack logs - it handles framework/service access internally
    auto result = commonHead::utilities::LogOperationUtils::packLogs(getCommonHeadFramework());

    fireNotification(&IMainWindowViewModelCallback::onLogsPackComplete, result.success, result.archivePath);
}
}
