#include <UIDataStruct/UIDataUtils.h>

#include <QQmlEngine>

#include <commonhead/viewModels/ClientInfoViewModel/ClientInfoModel.h>

#include <UIDataStruct/UIElementData.h>
#include <UIDataStruct/UILanguage.h>

void UIDataUtils::registerMetaObject()
{
    UIElementData::registerMetaObject();
    UILanguage::registerMetaObject();
}

commonHead::viewModels::model::LanguageType UIDataUtils::convertUILanguageToViewModelLanguage(UILanguage::LanguageType language)
{
    switch (language)
    {
    case UILanguage::LanguageType::LanguageType_ENGLISH:
        return commonHead::viewModels::model::LanguageType::ENGLISH;
    case UILanguage::LanguageType::LanguageType_CHINESE_SIMPLIFIED:
        return commonHead::viewModels::model::LanguageType::CHINESE_SIMPLIFIED;
    case UILanguage::LanguageType::LanguageType_CHINESE_TRADITIONAL:
        return commonHead::viewModels::model::LanguageType::CHINESE_TRADITIONAL;
    case UILanguage::LanguageType::LanguageType_FRENCH:
        return commonHead::viewModels::model::LanguageType::FRENCH;
    case UILanguage::LanguageType::LanguageType_GERMAN:
        return commonHead::viewModels::model::LanguageType::GERMAN;
    case UILanguage::LanguageType::LanguageType_ITALIAN:
        return commonHead::viewModels::model::LanguageType::ITALIAN;
    case UILanguage::LanguageType::LanguageType_SPANISH:
        return commonHead::viewModels::model::LanguageType::SPANISH;
    case UILanguage::LanguageType::LanguageType_PORTUGUESE:
        return commonHead::viewModels::model::LanguageType::PORTUGUESE;
    case UILanguage::LanguageType::LanguageType_JAPANESE:
        return commonHead::viewModels::model::LanguageType::JAPANESE;
    case UILanguage::LanguageType::LanguageType_KOREAN:
        return commonHead::viewModels::model::LanguageType::KOREAN;
    case UILanguage::LanguageType::LanguageType_RUSSIAN:
        return commonHead::viewModels::model::LanguageType::RUSSIAN;
    default:
        return commonHead::viewModels::model::LanguageType::ENGLISH;
    }
}

UILanguage::LanguageType UIDataUtils::convertViewModelLanguageToUILanguage(commonHead::viewModels::model::LanguageType language)
{
    switch (language)
    {
    case commonHead::viewModels::model::LanguageType::ENGLISH:
        return UILanguage::LanguageType::LanguageType_ENGLISH;
    case commonHead::viewModels::model::LanguageType::CHINESE_SIMPLIFIED:
        return UILanguage::LanguageType::LanguageType_CHINESE_SIMPLIFIED;
    case commonHead::viewModels::model::LanguageType::CHINESE_TRADITIONAL:
        return UILanguage::LanguageType::LanguageType_CHINESE_TRADITIONAL;
    case commonHead::viewModels::model::LanguageType::FRENCH:
        return UILanguage::LanguageType::LanguageType_FRENCH;
    case commonHead::viewModels::model::LanguageType::GERMAN:
        return UILanguage::LanguageType::LanguageType_GERMAN;
    case commonHead::viewModels::model::LanguageType::ITALIAN:
        return UILanguage::LanguageType::LanguageType_ITALIAN;
    case commonHead::viewModels::model::LanguageType::SPANISH:
        return UILanguage::LanguageType::LanguageType_SPANISH;
    case commonHead::viewModels::model::LanguageType::PORTUGUESE:
        return UILanguage::LanguageType::LanguageType_PORTUGUESE;
    case commonHead::viewModels::model::LanguageType::JAPANESE:
        return UILanguage::LanguageType::LanguageType_JAPANESE;
    case commonHead::viewModels::model::LanguageType::KOREAN:
        return UILanguage::LanguageType::LanguageType_KOREAN;
    case commonHead::viewModels::model::LanguageType::RUSSIAN:
        return UILanguage::LanguageType::LanguageType_RUSSIAN;
    default:
        return UILanguage::LanguageType::LanguageType_ENGLISH;
    }
}
