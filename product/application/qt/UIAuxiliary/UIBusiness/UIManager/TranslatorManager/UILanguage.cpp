#include <UIManager/UILanguage.h>
#include <QQmlEngine>
#include <commonhead/viewModels/ClientInfoViewModel/ClientInfoModel.h>

namespace UILanguage{

void registerMetaObject()
{
    qmlRegisterUncreatableMetaObject(
        UILanguage::staticMetaObject,
        "UILanguage",
        1, 0,
        "UILanguage",
        "Access to enums only"
    );
}

LanguageType convertFromViewModel(commonHead::viewModels::model::LanguageType language)
{
    switch (language)
    {
    case commonHead::viewModels::model::LanguageType::ENGLISH:
        return LanguageType::LanguageType_ENGLISH;
    case commonHead::viewModels::model::LanguageType::CHINESE_SIMPLIFIED:
        return LanguageType::LanguageType_CHINESE_SIMPLIFIED;
    case commonHead::viewModels::model::LanguageType::CHINESE_TRADITIONAL:
        return LanguageType::LanguageType_CHINESE_TRADITIONAL;
    case commonHead::viewModels::model::LanguageType::FRENCH:
        return LanguageType::LanguageType_FRENCH;
    case commonHead::viewModels::model::LanguageType::GERMAN:
        return LanguageType::LanguageType_GERMAN;
    case commonHead::viewModels::model::LanguageType::ITALIAN:
        return LanguageType::LanguageType_ITALIAN;
    case commonHead::viewModels::model::LanguageType::SPANISH:
        return LanguageType::LanguageType_SPANISH;
    case commonHead::viewModels::model::LanguageType::PORTUGUESE:
        return LanguageType::LanguageType_PORTUGUESE;
    case commonHead::viewModels::model::LanguageType::JAPANESE:
        return LanguageType::LanguageType_JAPANESE;
    case commonHead::viewModels::model::LanguageType::KOREAN:
        return LanguageType::LanguageType_KOREAN;
    case commonHead::viewModels::model::LanguageType::RUSSIAN:
        return LanguageType::LanguageType_RUSSIAN;
    default:
        return LanguageType::LanguageType_ENGLISH;
    }
}

commonHead::viewModels::model::LanguageType convertToViewModel(LanguageType language)
{
    switch (language)
    {
    case LanguageType::LanguageType_ENGLISH:
        return commonHead::viewModels::model::LanguageType::ENGLISH;
    case LanguageType::LanguageType_CHINESE_SIMPLIFIED:
        return commonHead::viewModels::model::LanguageType::CHINESE_SIMPLIFIED;
    case LanguageType::LanguageType_CHINESE_TRADITIONAL:
        return commonHead::viewModels::model::LanguageType::CHINESE_TRADITIONAL;
    case LanguageType::LanguageType_FRENCH:
        return commonHead::viewModels::model::LanguageType::FRENCH;
    case LanguageType::LanguageType_GERMAN:
        return commonHead::viewModels::model::LanguageType::GERMAN;
    case LanguageType::LanguageType_ITALIAN:
        return commonHead::viewModels::model::LanguageType::ITALIAN;
    case LanguageType::LanguageType_SPANISH:
        return commonHead::viewModels::model::LanguageType::SPANISH;
    case LanguageType::LanguageType_PORTUGUESE:
        return commonHead::viewModels::model::LanguageType::PORTUGUESE;
    case LanguageType::LanguageType_JAPANESE:
        return commonHead::viewModels::model::LanguageType::JAPANESE;
    case LanguageType::LanguageType_KOREAN:
        return commonHead::viewModels::model::LanguageType::KOREAN;
    case LanguageType::LanguageType_RUSSIAN:
        return commonHead::viewModels::model::LanguageType::RUSSIAN;
    default:
        return commonHead::viewModels::model::LanguageType::ENGLISH;
    }
}

}
