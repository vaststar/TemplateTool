#include <commonhead/viewmodels/ViewModelCore/IViewModel.h>

#include "LoggerDefine.h"

namespace commonHead::viewModels{

IViewModel::IViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
    : mCommonHeadFrameworkWptr(commonHeadFramework)
{
}

void IViewModel::initViewModel()
{
    std::call_once(mInitFlag, [this]() {
        VIEW_MODEL_CORE_LOG_DEBUG("start init ViewModel:" << getViewModelName());
        init();
        VIEW_MODEL_CORE_LOG_DEBUG("finish init ViewModel:" << getViewModelName());
    });
}

commonHead::ICommonHeadFrameworkWptr IViewModel::getCommonHeadFramework() const
{
    return mCommonHeadFrameworkWptr;
}
}
