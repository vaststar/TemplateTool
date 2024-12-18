#include <UIUtilities/LanguageUtils.h>
#include <commonhead/viewModels/ClientInfoViewModel/ClientInfoModel.h>
#include <UIManager/TranslatorManager.h>
#include "LoggerDefine.h"

commonHead::viewModels::model::LanguageType LanguageUtils::convertUILanguageToViewModelLanguage(UIManager::LanguageType language)
{
    switch (language)
    {
    case UIManager::LanguageType::ENGLISH:
        return commonHead::viewModels::model::LanguageType::ENGLISH;
    case UIManager::LanguageType::CHINESE_SIMPLIFIED:
        return commonHead::viewModels::model::LanguageType::CHINESE_SIMPLIFIED;
    case UIManager::LanguageType::CHINESE_TRADITIONAL:
        return commonHead::viewModels::model::LanguageType::CHINESE_TRADITIONAL;
    case UIManager::LanguageType::FRENCH:
        return commonHead::viewModels::model::LanguageType::FRENCH;
    case UIManager::LanguageType::GERMAN:
        return commonHead::viewModels::model::LanguageType::GERMAN;
    case UIManager::LanguageType::ITALIAN:
        return commonHead::viewModels::model::LanguageType::ITALIAN;
    case UIManager::LanguageType::SPANISH:
        return commonHead::viewModels::model::LanguageType::SPANISH;
    case UIManager::LanguageType::PORTUGUESE:
        return commonHead::viewModels::model::LanguageType::PORTUGUESE;
    case UIManager::LanguageType::JAPANESE:
        return commonHead::viewModels::model::LanguageType::JAPANESE;
    case UIManager::LanguageType::KOREAN:
        return commonHead::viewModels::model::LanguageType::KOREAN;
    case UIManager::LanguageType::RUSSIAN:
        return commonHead::viewModels::model::LanguageType::RUSSIAN;
    default:
        return commonHead::viewModels::model::LanguageType::ENGLISH;
    }
}

UIManager::LanguageType LanguageUtils::convertViewModelLanguageToUILanguage(commonHead::viewModels::model::LanguageType language)
{
    switch (language)
    {
    case commonHead::viewModels::model::LanguageType::ENGLISH:
        return UIManager::LanguageType::ENGLISH;
    case commonHead::viewModels::model::LanguageType::CHINESE_SIMPLIFIED:
        return UIManager::LanguageType::CHINESE_SIMPLIFIED;
    case commonHead::viewModels::model::LanguageType::CHINESE_TRADITIONAL:
        return UIManager::LanguageType::CHINESE_TRADITIONAL;
    case commonHead::viewModels::model::LanguageType::FRENCH:
        return UIManager::LanguageType::FRENCH;
    case commonHead::viewModels::model::LanguageType::GERMAN:
        return UIManager::LanguageType::GERMAN;
    case commonHead::viewModels::model::LanguageType::ITALIAN:
        return UIManager::LanguageType::ITALIAN;
    case commonHead::viewModels::model::LanguageType::SPANISH:
        return UIManager::LanguageType::SPANISH;
    case commonHead::viewModels::model::LanguageType::PORTUGUESE:
        return UIManager::LanguageType::PORTUGUESE;
    case commonHead::viewModels::model::LanguageType::JAPANESE:
        return UIManager::LanguageType::JAPANESE;
    case commonHead::viewModels::model::LanguageType::KOREAN:
        return UIManager::LanguageType::KOREAN;
    case commonHead::viewModels::model::LanguageType::RUSSIAN:
        return UIManager::LanguageType::RUSSIAN;
    default:
        return UIManager::LanguageType::ENGLISH;
    }
}