#include <commonHead/viewModels/IViewModel/IViewModel.h>

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>

namespace commonHead::viewModels{

IViewModel::IViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
    : mCommonHeadFrameworkWptr(commonHeadFramework)
{
}

void IViewModel::initViewModel()
{
    COMMONHEAD_LOG_DEBUG("start init ViewModel:" << getViewModelName());
    init();
    COMMONHEAD_LOG_DEBUG("finish init ViewModel:" << getViewModelName());
}

commonHead::ICommonHeadFrameworkWptr IViewModel::getCommonHeadFramework() const
{
    return mCommonHeadFrameworkWptr;
}
}