#include "PageViews/SettingsPage/include/LanguageSettingsController.h"
#include "LoggerDefine/LoggerDefine.h"

#include <commonHead/viewModels/ClientInfoViewModel/IClientInfoViewModel.h>
#include <commonHead/viewModels/ClientInfoViewModel/ClientInfoModel.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>
#include <AppContext/AppContext.h>
#include <UIManager/IUIManagerProvider.h>
#include <TranslatorManager/ITranslatorManager.h>
#include <TranslatorManager/UILanguage.h>

LanguageSettingsController::LanguageSettingsController(QObject* parent)
    : UIViewController(parent)
{
    UIVIEW_LOG_DEBUG("create LanguageSettingsController");
}

void LanguageSettingsController::init()
{
    UIVIEW_LOG_DEBUG("LanguageSettingsController::init");

    m_clientInfoViewModel = getAppContext()->getViewModelFactory()->createClientInfoViewModelInstance();
    m_clientInfoViewModel->initViewModel();

    buildLanguageData();

    UIVIEW_LOG_DEBUG("LanguageSettingsController::init done");
}

QStringList LanguageSettingsController::getSupportedLanguages() const
{
    return m_supportedLanguages;
}

int LanguageSettingsController::getCurrentLanguageIndex() const
{
    return m_currentLanguageIndex;
}

void LanguageSettingsController::setLanguage(int index)
{
    if (index < 0 || index >= static_cast<int>(m_languageValues.size()))
        return;

    auto langType = static_cast<commonHead::viewModels::model::LanguageType>(m_languageValues[static_cast<size_t>(index)]);
    UIVIEW_LOG_DEBUG("setLanguage index:" << index << " langType:" << m_languageValues[static_cast<size_t>(index)]);

    m_currentLanguageIndex = index;
    emit currentLanguageIndexChanged();

    m_clientInfoViewModel->setApplicationLanguage(langType);
    getAppContext()->getManagerProvider()->getTranslatorManager()->loadTranslation(UILanguage::convertFromViewModel(langType));
}

void LanguageSettingsController::buildLanguageData()
{
    if (!m_clientInfoViewModel)
        return;

    m_supportedLanguages.clear();
    m_languageValues.clear();
    auto languages = m_clientInfoViewModel->getSupportedLanguages();
    auto currentLang = m_clientInfoViewModel->getApplicationLanguage();
    for (size_t i = 0; i < languages.size(); ++i)
    {
        m_languageValues.push_back(static_cast<int>(languages[i]));
        m_supportedLanguages.append(languageTypeToDisplayString(static_cast<int>(languages[i])));
        if (languages[i] == currentLang)
            m_currentLanguageIndex = static_cast<int>(i);
    }
    emit supportedLanguagesChanged();
    emit currentLanguageIndexChanged();
}

QString LanguageSettingsController::languageTypeToDisplayString(int languageType) const
{
    using LanguageType = commonHead::viewModels::model::LanguageType;
    switch (static_cast<LanguageType>(languageType))
    {
    case LanguageType::ENGLISH:              return QStringLiteral("English");
    case LanguageType::CHINESE_SIMPLIFIED:   return QStringLiteral("简体中文");
    case LanguageType::CHINESE_TRADITIONAL:  return QStringLiteral("繁體中文");
    case LanguageType::FRENCH:               return QStringLiteral("Français");
    case LanguageType::GERMAN:               return QStringLiteral("Deutsch");
    case LanguageType::ITALIAN:              return QStringLiteral("Italiano");
    case LanguageType::SPANISH:              return QStringLiteral("Español");
    case LanguageType::PORTUGUESE:           return QStringLiteral("Português");
    case LanguageType::JAPANESE:             return QStringLiteral("日本語");
    case LanguageType::KOREAN:               return QStringLiteral("한국어");
    case LanguageType::RUSSIAN:              return QStringLiteral("Русский");
    default:                                 return QStringLiteral("Unknown");
    }
}
