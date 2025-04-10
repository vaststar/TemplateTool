#include "MainWindowViewModel.h"

#include <ucf/CoreFramework/ICoreFramework.h>

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>

namespace commonHead::viewModels{
std::shared_ptr<IMainWindowViewModel> IMainWindowViewModel::createInstance(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<MainWindowViewModel>(commonHeadFramework);
}

MainWindowViewModel::MainWindowViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
    : mCommonHeadFrameworkWptr(commonHeadFramework)
{
    COMMONHEAD_LOG_DEBUG("create MainWindowViewModel");
}

std::string MainWindowViewModel::getViewModelName() const
{
    return "MainWindowViewModel";
}
}