#pragma once

#include <memory>

#include <commonHead/commonHeadUtils/VMNotificationHelper/VMNotificationHelper.h>
#include <commonHead/viewModels/MainWindowViewModel/IMainWindowViewModel.h>

namespace commonHead::viewModels{
class MainWindowViewModel: public virtual IMainWindowViewModel, 
                           public virtual commonHead::utilities::VMNotificationHelper<IMainWindowViewModelCallback>,
                           public std::enable_shared_from_this<MainWindowViewModel>
{
public:
    MainWindowViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
    virtual std::string getViewModelName() const override;

private:
    commonHead::ICommonHeadFrameworkWptr mCommonHeadFrameworkWptr;
};
}