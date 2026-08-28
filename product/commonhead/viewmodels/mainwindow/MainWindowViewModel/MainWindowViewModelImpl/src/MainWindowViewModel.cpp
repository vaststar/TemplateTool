#include "MainWindowViewModel.h"
#include "LoggerDefine.h"

#include <commonhead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonhead/ServiceLocator/IServiceLocator.h>
#include <commonhead/viewmodels/ViewModelUtils/LogOperationUtils.h>
#include <commonhead/viewmodels/MainWindowViewModel/MainWindowViewModelCreator.h>
#include <ucf/services/InvocationService/IInvocationService.h>

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
    MAIN_WINDOW_VIEW_MODEL_LOG_DEBUG("MainWindowViewModel constructed, address: " << this);
}

MainWindowViewModel::~MainWindowViewModel()
{
    MAIN_WINDOW_VIEW_MODEL_LOG_DEBUG("MainWindowViewModel destroying, address: " << this);
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
