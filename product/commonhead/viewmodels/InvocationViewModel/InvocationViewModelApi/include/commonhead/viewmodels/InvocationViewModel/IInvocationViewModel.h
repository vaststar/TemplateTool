#pragma once

#include <string>
#include <vector>

#include <commonhead/utilities/VMNotificationHelper/IVMNotificationHelper.h>
#include <commonhead/viewmodels/ViewModelCore/IViewModel.h>

namespace commonHead::viewModels{

class IInvocationViewModelCallback
{
public:
    IInvocationViewModelCallback() = default;
    IInvocationViewModelCallback(const IInvocationViewModelCallback&) = delete;
    IInvocationViewModelCallback(IInvocationViewModelCallback&&) = delete;
    IInvocationViewModelCallback& operator=(const IInvocationViewModelCallback&) = delete;
    IInvocationViewModelCallback& operator=(IInvocationViewModelCallback&&) = delete;
    virtual ~IInvocationViewModelCallback() = default;
};

class IInvocationViewModel: public IViewModel, public virtual commonHead::utilities::IVMNotificationHelper<IInvocationViewModelCallback>
{
public:
    using IViewModel::IViewModel;
    IInvocationViewModel(const IInvocationViewModel&) = delete;
    IInvocationViewModel(IInvocationViewModel&&) = delete;
    IInvocationViewModel& operator=(const IInvocationViewModel&) = delete;
    IInvocationViewModel& operator=(IInvocationViewModel&&) = delete;
    virtual ~IInvocationViewModel() = default;
public:
    virtual void processStartupParameters() = 0;
    [[nodiscard]] virtual std::vector<std::string> getStartupParameters() const = 0;
    virtual void processCommandMessage(const std::string& message) = 0;
};

}
