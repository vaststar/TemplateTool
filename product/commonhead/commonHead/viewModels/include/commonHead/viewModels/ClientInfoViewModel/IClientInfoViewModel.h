#pragma once

#include <string>
#include <vector>
#include <memory>

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>
#include <commonHead/commonHeadUtils/VMNotificationHelper/IVMNotificationHelper.h>

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWptr = std::weak_ptr<ICommonHeadFramework>;
}

namespace commonHead::viewModels::model{
    enum class LanguageType;
    enum class ThemeType;
}

namespace commonHead::viewModels{
class COMMONHEAD_EXPORT IClientInfoViewModelCallback
{
public:
    virtual ~IClientInfoViewModelCallback() = default;
};

class COMMONHEAD_EXPORT IClientInfoViewModel: public virtual commonHead::utilities::IVMNotificationHelper<IClientInfoViewModelCallback>
{
public:
    virtual std::string getViewModelName() const = 0;

    virtual std::string getApplicationVersion() const = 0;

    virtual commonHead::viewModels::model::LanguageType getApplicationLanguage() const = 0;
    virtual void setApplicationLanguage(commonHead::viewModels::model::LanguageType language) = 0;
    virtual std::vector<commonHead::viewModels::model::LanguageType> getSupportedLanguages() const = 0;

    virtual void setCurrentThemeType(commonHead::viewModels::model::ThemeType themeType) = 0;
    virtual commonHead::viewModels::model::ThemeType getCurrentThemeType() const = 0;
    virtual std::vector<commonHead::viewModels::model::ThemeType> getSupportedThemeTypes() const = 0;
public:
    static std::shared_ptr<IClientInfoViewModel> createInstance(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
};
}