#include <commonhead/viewmodels/ViewModelCore/IViewModel.h>

#include "LoggerDefine.h"

namespace commonHead::viewModels {

IViewModel::IViewModel(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
    : mCommonHeadFrameworkWptr(commonHeadFramework)
{
}

void IViewModel::initViewModel()
{
    std::call_once(mInitFlag, [this]() {
        const auto viewModelName = getViewModelName();
        const auto* objectAddress = dynamic_cast<const void*>(this);

        VIEW_MODEL_CORE_LOG_DEBUG(
            viewModelName
            << " initialization started, address: "
            << objectAddress);

        init();

        VIEW_MODEL_CORE_LOG_DEBUG(
            viewModelName
            << " initialization finished, address: "
            << objectAddress);
    });
}

commonHead::ICommonHeadFrameworkWptr IViewModel::getCommonHeadFramework() const
{
    return mCommonHeadFrameworkWptr;
}

} // namespace commonHead::viewModels
