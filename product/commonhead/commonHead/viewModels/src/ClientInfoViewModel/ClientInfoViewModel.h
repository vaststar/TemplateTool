#pragma once

#include <memory>

#include <commonHead/commonHeadUtils/VMNotificationHelper/VMNotificationHelper.h>
#include <commonHead/viewModels/ClientInfoViewModel/IClientInfoViewModel.h>

namespace commonHead::viewModels::model{
    enum class LanguageType;
    enum class ThemeType;
}

namespace ucf::service::model{
    enum class LanguageType;
    enum class ThemeType;
}

namespace commonHead::viewModels{
class ClientInfoViewModel: public virtual IClientInfoViewModel, 
                           public virtual commonHead::utilities::VMNotificationHelper<IClientInfoViewModelCallback>,
                           public std::enable_shared_from_this<ClientInfoViewModel>
{
public:
    ClientInfoViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
    ClientInfoViewModel() = default;
    ClientInfoViewModel(const ClientInfoViewModel&) = delete;
    ClientInfoViewModel(ClientInfoViewModel&&) = delete;
    ClientInfoViewModel& operator=(const ClientInfoViewModel&) = delete;
    ClientInfoViewModel& operator=(ClientInfoViewModel&&) = delete;
    ~ClientInfoViewModel() = default;
public:
    virtual std::string getViewModelName() const override;
    virtual std::string getApplicationVersion() const override;
    
    virtual commonHead::viewModels::model::LanguageType getApplicationLanguage() const override;
    virtual void setApplicationLanguage(commonHead::viewModels::model::LanguageType language) override;
    virtual std::vector<commonHead::viewModels::model::LanguageType> getSupportedLanguages() const override;

    virtual void setCurrentThemeType(commonHead::viewModels::model::ThemeType themeType) override;
    virtual commonHead::viewModels::model::ThemeType getCurrentThemeType() const override;
    virtual std::vector<commonHead::viewModels::model::ThemeType> getSupportedThemeTypes() const override;

private:
    commonHead::viewModels::model::LanguageType convertServiceLanguageToModelLanguage(ucf::service::model::LanguageType language) const;
    ucf::service::model::LanguageType convertModelLanguageToServiceLanguage(commonHead::viewModels::model::LanguageType language) const;

    
    commonHead::viewModels::model::ThemeType convertServiceThemeTypeToModelThemeType(ucf::service::model::ThemeType theme) const;
    ucf::service::model::ThemeType convertModelThemeTypeToServiceThemeType(commonHead::viewModels::model::ThemeType theme) const;

private:
    commonHead::ICommonHeadFrameworkWptr mCommonHeadFrameworkWptr;
};
}