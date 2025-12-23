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
    explicit AppUIViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
    AppUIViewModel(const AppUIViewModel&) = delete;
    AppUIViewModel(AppUIViewModel&&) = delete;
    AppUIViewModel& operator=(const AppUIViewModel&) = delete;
    AppUIViewModel& operator=(AppUIViewModel&&) = delete;
    ~AppUIViewModel() = default;
public:
    virtual std::string getViewModelName() const override;
    virtual void initApplication() override;
protected:
    void init();
private:    
    void initDatabase();
};
}