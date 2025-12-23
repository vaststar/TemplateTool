#pragma once

#include <vector>
#include <memory>
#include <string>

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>
#include <commonHead/commonHeadUtils/VMNotificationHelper/IVMNotificationHelper.h>
#include <commonHead/viewModels/IViewModel/IViewModel.h>

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWptr = std::weak_ptr<ICommonHeadFramework>;
}

namespace commonHead::viewModels{
class COMMONHEAD_EXPORT IInvocationViewModelCallback
{
public:
    IInvocationViewModelCallback() = default;
    IInvocationViewModelCallback(const IInvocationViewModelCallback&) = delete;
    IInvocationViewModelCallback(IInvocationViewModelCallback&&) = delete;
    IInvocationViewModelCallback& operator=(const IInvocationViewModelCallback&) = delete;
    IInvocationViewModelCallback& operator=(IInvocationViewModelCallback&&) = delete;
    virtual ~IInvocationViewModelCallback() = default;
};

class COMMONHEAD_EXPORT IInvocationViewModel: public IViewModel, public virtual commonHead::utilities::IVMNotificationHelper<IInvocationViewModelCallback>
{
public:
    using IViewModel::IViewModel;
    IInvocationViewModel(const IInvocationViewModel&) = delete;
    IInvocationViewModel(IInvocationViewModel&&) = delete;
    IInvocationViewModel& operator=(const IInvocationViewModel&) = delete;
    IInvocationViewModel& operator=(IInvocationViewModel&&) = delete;
    virtual ~IInvocationViewModel() = default;
public:
    virtual std::string getViewModelName() const = 0;

    virtual void processStartupParameters() = 0;
    [[nodiscard]] virtual std::vector<std::string> getStartupParameters() const = 0;
    virtual void processCommandMessage(const std::string& message) = 0;
public:
    static std::shared_ptr<IInvocationViewModel> createInstance(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
};
}