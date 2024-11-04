#pragma once

#include <memory>

#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>
#include <commonHead/viewModels/MainWindowViewModel/IMainWindowViewModel.h>

namespace commonHead::viewModels{
class MainWindowViewModel: public virtual IMainWindowViewModel, 
                           public virtual ucf::utilities::NotificationHelper<IMainWindowViewModelCallback>,
                           public std::enable_shared_from_this<MainWindowViewModel>
{
public:
    MainWindowViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
    virtual std::string getViewModelName() const override;

    virtual void initDatabase() const override;
    virtual void openCamera() override;
private:
    commonHead::ICommonHeadFrameworkWptr mCommonHeadFrameworkWptr;
};
}