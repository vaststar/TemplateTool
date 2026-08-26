#pragma once

#include <string>
#include <commonhead/utilities/VMNotificationHelper/IVMNotificationHelper.h>
#include <commonhead/viewmodels/ViewModelCore/IViewModel.h>

namespace commonHead::viewModels{
class IAppUIViewModelCallback
{
public:
    IAppUIViewModelCallback() = default;
    IAppUIViewModelCallback(const IAppUIViewModelCallback&) = delete;
    IAppUIViewModelCallback(IAppUIViewModelCallback&&) = delete;
    IAppUIViewModelCallback& operator=(const IAppUIViewModelCallback&) = delete;
    IAppUIViewModelCallback& operator=(IAppUIViewModelCallback&&) = delete;
    virtual ~IAppUIViewModelCallback() = default;
public:
    virtual void onAppConfigInitialized() = 0;
};

class IAppUIViewModel: public IViewModel, public virtual commonHead::utilities::IVMNotificationHelper<IAppUIViewModelCallback>
{
public:
    using IViewModel::IViewModel;
    IAppUIViewModel(const IAppUIViewModel&) = delete;
    IAppUIViewModel(IAppUIViewModel&&) = delete;
    IAppUIViewModel& operator=(const IAppUIViewModel&) = delete;
    IAppUIViewModel& operator=(IAppUIViewModel&&) = delete;
    virtual ~IAppUIViewModel() = default;
public:
    virtual std::string getViewModelName() const = 0;
};
}
