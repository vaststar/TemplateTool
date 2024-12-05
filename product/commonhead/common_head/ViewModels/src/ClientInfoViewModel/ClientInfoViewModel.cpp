#include "ClientInfoViewModel.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Services/ClientInfoService/ClientInfoModel.h>
#include <ucf/Services/ClientInfoService/IClientInfoService.h>

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>

#include <commonHead/viewModels/ClientInfoViewModel/ClientInfoModel.h>

namespace commonHead::viewModels{
std::shared_ptr<IClientInfoViewModel> IClientInfoViewModel::createInstance(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<ClientInfoViewModel>(commonHeadFramework);
}

ClientInfoViewModel::ClientInfoViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
    : mCommonHeadFrameworkWptr(commonHeadFramework)
{
    COMMONHEAD_LOG_DEBUG("create ClientInfoViewModel");
}

std::string ClientInfoViewModel::getViewModelName() const
{
    return "ClientInfoViewModel";
}

std::string ClientInfoViewModel::getApplicationVersion() const
{
    if (auto coreFramework = mCommonHeadFrameworkWptr.lock()->getCoreFramework().lock())
    {
        if (auto service = coreFramework->getService<ucf::service::IClientInfoService>().lock())
        {
            return service->getApplicationVersion().toString();
        }
    }
    return "0.0.0";
}

commonHead::viewModels::model::LanguageType ClientInfoViewModel::getApplicationLanguage() const
{
    if (auto coreFramework = mCommonHeadFrameworkWptr.lock()->getCoreFramework().lock())
    {
        if (auto service = coreFramework->getService<ucf::service::IClientInfoService>().lock())
        {
            return convertServiceLanguageToModelLanguage(service->getApplicationLanguage());
        }
    }
    return commonHead::viewModels::model::LanguageType::ENGLISH;

}

void ClientInfoViewModel::setApplicationLanguage(commonHead::viewModels::model::LanguageType language)
{
    if (auto coreFramework = mCommonHeadFrameworkWptr.lock()->getCoreFramework().lock())
    {
        if (auto service = coreFramework->getService<ucf::service::IClientInfoService>().lock())
        {
            return service->setApplicationLanguage(convertModelLanguageToServiceLanguage(language));
        }
    }
}

commonHead::viewModels::model::LanguageType ClientInfoViewModel::convertServiceLanguageToModelLanguage(ucf::service::model::LanguageType language) const
{
    switch (language)
    {
    case ucf::service::model::LanguageType::ENGLISH:
        return commonHead::viewModels::model::LanguageType::ENGLISH;
    case ucf::service::model::LanguageType::CHINESE_SIMPLIFIED:
        return commonHead::viewModels::model::LanguageType::CHINESE_SIMPLIFIED;
    case ucf::service::model::LanguageType::CHINESE_TRADITIONAL:
        return commonHead::viewModels::model::LanguageType::CHINESE_TRADITIONAL;
    case ucf::service::model::LanguageType::FRENCH:
        return commonHead::viewModels::model::LanguageType::FRENCH;
    case ucf::service::model::LanguageType::GERMAN:
        return commonHead::viewModels::model::LanguageType::GERMAN;
    case ucf::service::model::LanguageType::ITALIAN:
        return commonHead::viewModels::model::LanguageType::ITALIAN;
    case ucf::service::model::LanguageType::SPANISH:
        return commonHead::viewModels::model::LanguageType::SPANISH;
    case ucf::service::model::LanguageType::PORTUGUESE:
        return commonHead::viewModels::model::LanguageType::PORTUGUESE;
    case ucf::service::model::LanguageType::JAPANESE:
        return commonHead::viewModels::model::LanguageType::JAPANESE;
    case ucf::service::model::LanguageType::KOREAN:
        return commonHead::viewModels::model::LanguageType::KOREAN;
    case ucf::service::model::LanguageType::RUSSIAN:
        return commonHead::viewModels::model::LanguageType::RUSSIAN;
    default:
        return commonHead::viewModels::model::LanguageType::ENGLISH;
    }
}

ucf::service::model::LanguageType ClientInfoViewModel::convertModelLanguageToServiceLanguage(commonHead::viewModels::model::LanguageType language) const
{
    switch (language)
    {
    case commonHead::viewModels::model::LanguageType::ENGLISH:
        return ucf::service::model::LanguageType::ENGLISH;
    case commonHead::viewModels::model::LanguageType::CHINESE_SIMPLIFIED:
        return ucf::service::model::LanguageType::CHINESE_SIMPLIFIED;
    case commonHead::viewModels::model::LanguageType::CHINESE_TRADITIONAL:
        return ucf::service::model::LanguageType::CHINESE_TRADITIONAL;
    case commonHead::viewModels::model::LanguageType::FRENCH:
        return ucf::service::model::LanguageType::FRENCH;
    case commonHead::viewModels::model::LanguageType::GERMAN:
        return ucf::service::model::LanguageType::GERMAN;
    case commonHead::viewModels::model::LanguageType::ITALIAN:
        return ucf::service::model::LanguageType::ITALIAN;
    case commonHead::viewModels::model::LanguageType::SPANISH:
        return ucf::service::model::LanguageType::SPANISH;
    case commonHead::viewModels::model::LanguageType::PORTUGUESE:
        return ucf::service::model::LanguageType::PORTUGUESE;
    case commonHead::viewModels::model::LanguageType::JAPANESE:
        return ucf::service::model::LanguageType::JAPANESE;
    case commonHead::viewModels::model::LanguageType::KOREAN:
        return ucf::service::model::LanguageType::KOREAN;
    case commonHead::viewModels::model::LanguageType::RUSSIAN:
        return ucf::service::model::LanguageType::RUSSIAN;
    default:
        return ucf::service::model::LanguageType::ENGLISH;
    }
}

std::vector<commonHead::viewModels::model::LanguageType> ClientInfoViewModel::getSupportedLanguages() const
{
    if (auto coreFramework = mCommonHeadFrameworkWptr.lock()->getCoreFramework().lock())
    {
        if (auto service = coreFramework->getService<ucf::service::IClientInfoService>().lock())
        {
            auto supportedLanguages = service->getSupportedLanguages();
            std::vector<commonHead::viewModels::model::LanguageType> results;
            std::transform(supportedLanguages.begin(), supportedLanguages.end(), std::back_inserter(results), [this](ucf::service::model::LanguageType la){
                return convertServiceLanguageToModelLanguage(la);
            });
            return results;
        }
    }
    return {};
}
}