#pragma once

#include <vector>
#include <memory>

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>
#include <commonHead/commonHeadUtils/VMNotificationHelper/IVMNotificationHelper.h>
#include <commonHead/viewModels/IViewModel/IViewModel.h>

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWptr = std::weak_ptr<ICommonHeadFramework>;
}

namespace commonHead::viewModels{
class COMMONHEAD_EXPORT IMainWindowViewModelCallback
{
public:
    IMainWindowViewModelCallback() = default;
    IMainWindowViewModelCallback(const IMainWindowViewModelCallback&) = delete;
    IMainWindowViewModelCallback(IMainWindowViewModelCallback&&) = delete;
    IMainWindowViewModelCallback& operator=(const IMainWindowViewModelCallback&) = delete;
    IMainWindowViewModelCallback& operator=(IMainWindowViewModelCallback&&) = delete;
    virtual ~IMainWindowViewModelCallback() = default;
public:
    virtual void onActivateMainWindow() = 0;
};

class COMMONHEAD_EXPORT IMainWindowViewModel: public IViewModel, public virtual commonHead::utilities::IVMNotificationHelper<IMainWindowViewModelCallback>
{
public:
    using IViewModel::IViewModel;
    IMainWindowViewModel(const IMainWindowViewModel&) = delete;
    IMainWindowViewModel(IMainWindowViewModel&&) = delete;
    IMainWindowViewModel& operator=(const IMainWindowViewModel&) = delete;
    IMainWindowViewModel& operator=(IMainWindowViewModel&&) = delete;
    virtual ~IMainWindowViewModel() = default;
public:
    static std::shared_ptr<IMainWindowViewModel> createInstance(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
};
}