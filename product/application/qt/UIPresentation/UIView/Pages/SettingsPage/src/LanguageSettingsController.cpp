#include "SettingsPage/LanguageSettingsController.h"
#include "LoggerDefine.h"

#include <algorithm>

#include <commonhead/viewmodels/ClientInfoViewModel/IClientInfoViewModel.h>
#include <commonhead/viewmodels/ClientInfoViewModel/ClientInfoModel.h>
#include <commonhead/viewmodels/ViewModelFactory/IViewModelFactory.h>
#include <AppContext/AppContext.h>
#include <UIManager/IUIManagerProvider.h>
#include <TranslatorManager/ITranslatorManager.h>
#include <TranslatorManager/UILanguage.h>

LanguageSettingsController::LanguageSettingsController(QObject* parent)
    : UIViewController(parent)
{
    UIVIEW_LOG_DEBUG("LanguageSettingsController constructed, address: " << this);
}

LanguageSettingsController::~LanguageSettingsController()
{
    UIVIEW_LOG_DEBUG("LanguageSettingsController destroying, address: " << this);
}

void LanguageSettingsController::init()
{
    m_clientInfoViewModel = getViewModelFactory()->createClientInfoViewModelInstance();
    m_clientInfoViewModel->initViewModel();

    buildLanguageData();
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
    {
        UIVIEW_LOG_WARN("invalid language index:" << index);
        return;
    }

    if (!m_clientInfoViewModel)
    {
        UIVIEW_LOG_WARN("cannot set language because ClientInfoViewModel is unavailable");
        return;
    }

    auto translatorManager = getTranslatorManager();
    if (!translatorManager)
    {
        UIVIEW_LOG_WARN("cannot set language because TranslatorManager is unavailable");
        return;
    }

    const auto language = static_cast<commonHead::viewModels::model::LanguageType>(
        m_languageValues[static_cast<std::size_t>(index)]);
    const auto uiLanguage = UILanguage::convertFromViewModel(language);
    UIVIEW_LOG_DEBUG("setLanguage index:" << index
        << " langType:" << static_cast<int>(language));

    const auto result = translatorManager->loadTranslation(uiLanguage);
    if (!UIManager::isTranslationLoadSuccessful(result))
    {
        UIVIEW_LOG_WARN("setLanguage failed, index:" << index
            << ", langType:" << static_cast<int>(language)
            << ", result:" << static_cast<int>(result));

        // The ComboBox has already selected the requested item. Re-emit the
        // unchanged controller value so QML restores the previous selection.
        emit currentLanguageIndexChanged();
        return;
    }

    if (m_currentLanguageIndex != index)
    {
        m_currentLanguageIndex = index;
        emit currentLanguageIndexChanged();
    }

    // Persist only after the presentation layer accepted the language.
    m_clientInfoViewModel->setApplicationLanguage(language);
}

void LanguageSettingsController::buildLanguageData()
{
    if (!m_clientInfoViewModel)
    {
        return;
    }

    auto translatorManager = getTranslatorManager();
    if (!translatorManager)
    {
        UIVIEW_LOG_WARN("cannot build language data because TranslatorManager is unavailable");
        return;
    }

    m_supportedLanguages.clear();
    m_languageValues.clear();
    m_currentLanguageIndex = 0;

    const auto serviceLanguages = m_clientInfoViewModel->getSupportedLanguages();
    const auto packagedLanguages = translatorManager->getAvailableLanguages();
    const auto currentLanguage = m_clientInfoViewModel->getApplicationLanguage();

    for (const auto language : serviceLanguages)
    {
        const auto uiLanguage = UILanguage::convertFromViewModel(language);
        if (std::find(packagedLanguages.begin(), packagedLanguages.end(), uiLanguage) ==
            packagedLanguages.end())
        {
            continue;
        }

        const auto languageValue = static_cast<int>(language);
        const auto languageIndex = static_cast<int>(m_languageValues.size());
        m_languageValues.push_back(languageValue);
        m_supportedLanguages.append(languageTypeToDisplayString(languageValue));
        if (language == currentLanguage)
        {
            m_currentLanguageIndex = languageIndex;
        }
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
