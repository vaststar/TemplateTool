#pragma once

#include <memory>

#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>
#include <commonHead/viewModels/AppUIViewModel/IAppUIViewModel.h>

namespace commonHead::viewModels::model{
    enum class LanguageType;
}

namespace ucf::service::model{
    enum class LanguageType;
}

namespace commonHead::viewModels{
class AppUIViewModel: public virtual IAppUIViewModel, 
                            public virtual ucf::utilities::NotificationHelper<IAppUIViewModelCallback>,
                            public std::enable_shared_from_this<AppUIViewModel>
{
public:
    AppUIViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
    virtual std::string getViewModelName() const override;
    virtual void initDatabase() override;
private:
    commonHead::ICommonHeadFrameworkWptr mCommonHeadFrameworkWptr;
};
}