#pragma once

#include <memory>

#include <commonHead/commonHeadUtils/VMNotificationHelper/VMNotificationHelper.h>
#include <commonHead/viewModels/AppUIViewModel/IAppUIViewModel.h>

namespace commonHead::viewModels::model{
    enum class LanguageType;
}

namespace ucf::service::model{
    enum class LanguageType;
}

namespace commonHead::viewModels{
class AppUIViewModel: public virtual IAppUIViewModel, 
                      public virtual commonHead::utilities::VMNotificationHelper<IAppUIViewModelCallback>,
                      public std::enable_shared_from_this<AppUIViewModel>
{
public:
    AppUIViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
    virtual std::string getViewModelName() const override;
    virtual void initApplication() override;
private:    
    void initDatabase();
private:
    commonHead::ICommonHeadFrameworkWptr mCommonHeadFrameworkWptr;
};
}