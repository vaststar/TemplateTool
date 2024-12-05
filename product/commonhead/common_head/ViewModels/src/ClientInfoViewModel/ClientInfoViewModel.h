#pragma once

#include <memory>

#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>
#include <commonHead/viewModels/ClientInfoViewModel/IClientInfoViewModel.h>

namespace commonHead::viewModels::model{
    enum class LanguageType;
}

namespace ucf::service::model{
    enum class LanguageType;
}

namespace commonHead::viewModels{
class ClientInfoViewModel: public virtual IClientInfoViewModel, 
                            public virtual ucf::utilities::NotificationHelper<IClientInfoViewModelCallback>,
                            public std::enable_shared_from_this<ClientInfoViewModel>
{
public:
    ClientInfoViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
    virtual std::string getViewModelName() const override;
    virtual std::string getApplicationVersion() const override;
    virtual commonHead::viewModels::model::LanguageType getApplicationLanguage() const override;
    virtual void setApplicationLanguage(commonHead::viewModels::model::LanguageType language) override;
    virtual std::vector<commonHead::viewModels::model::LanguageType> getSupportedLanguages() const override;
private:
    commonHead::viewModels::model::LanguageType convertServiceLanguageToModelLanguage(ucf::service::model::LanguageType language) const;
    ucf::service::model::LanguageType convertModelLanguageToServiceLanguage(commonHead::viewModels::model::LanguageType language) const;

private:
    commonHead::ICommonHeadFrameworkWptr mCommonHeadFrameworkWptr;
};
}