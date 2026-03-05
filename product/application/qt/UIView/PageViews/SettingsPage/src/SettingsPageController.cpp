#include "PageViews/SettingsPage/include/SettingsPageController.h"
#include "LoggerDefine/LoggerDefine.h"

#include <commonHead/viewModels/ClientInfoViewModel/IClientInfoViewModel.h>
#include <commonHead/viewModels/ClientInfoViewModel/ClientInfoModel.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>
#include <AppContext/AppContext.h>
#include <UIManager/IUIManagerProvider.h>
#include <UIManager/ITranslatorManager.h>
#include <UIManager/UILanguage.h>
#include <UIResourceLoaderManager/IUIResourceLoaderManager.h>

SettingsPageController::SettingsPageController(QObject* parent)
    : UIViewController(parent)
{
    UIVIEW_LOG_DEBUG("create SettingsPageController");
}

void SettingsPageController::init()
{
    UIVIEW_LOG_DEBUG("SettingsPageController::init");

    mClientInfoViewModel = getAppContext()->getViewModelFactory()->createClientInfoViewModelInstance();
    mClientInfoViewModel->initViewModel();

    buildNavModel();
    buildThemeAndLanguageData();

    UIVIEW_LOG_DEBUG("SettingsPageController::init done");
}

SettingsNavModel* SettingsPageController::getNavModel() const
{
    return mNavModel;
}

QStringList SettingsPageController::getSupportedThemes() const
{
    return mSupportedThemes;
}

QStringList SettingsPageController::getSupportedLanguages() const
{
    return mSupportedLanguages;
}

int SettingsPageController::getCurrentThemeIndex() const
{
    return mCurrentThemeIndex;
}

int SettingsPageController::getCurrentLanguageIndex() const
{
    return mCurrentLanguageIndex;
}

void SettingsPageController::setTheme(int index)
{
    if (index < 0 || index >= static_cast<int>(mThemeValues.size()))
        return;

    auto themeType = static_cast<commonHead::viewModels::model::ThemeType>(mThemeValues[static_cast<size_t>(index)]);
    UIVIEW_LOG_DEBUG("setTheme index:" << index << " themeType:" << mThemeValues[static_cast<size_t>(index)]);

    mClientInfoViewModel->setCurrentThemeType(themeType);
    getAppContext()->getManagerProvider()->getUIResourceLoaderManager()->notifyThemeChanged();
    mCurrentThemeIndex = index;
    emit currentThemeIndexChanged();
}

void SettingsPageController::setLanguage(int index)
{
    if (index < 0 || index >= static_cast<int>(mLanguageValues.size()))
        return;

    auto langType = static_cast<commonHead::viewModels::model::LanguageType>(mLanguageValues[static_cast<size_t>(index)]);
    UIVIEW_LOG_DEBUG("setLanguage index:" << index << " langType:" << mLanguageValues[static_cast<size_t>(index)]);

    mClientInfoViewModel->setApplicationLanguage(langType);
    getAppContext()->getManagerProvider()->getTranslatorManager()->loadTranslation(UILanguage::convertFromViewModel(langType));
    mCurrentLanguageIndex = index;
    emit currentLanguageIndexChanged();
}

void SettingsPageController::buildNavModel()
{
    mNavModel = new SettingsNavModel(this);
    mNavModel->setItems({
        { 0, tr("General"), tr("Appearance"), QString() },
        { 1, tr("General"), tr("Language"), QString() }
    });
    emit navModelChanged();
}

void SettingsPageController::buildThemeAndLanguageData()
{
    if (!mClientInfoViewModel)
        return;

    // Themes
    mSupportedThemes.clear();
    mThemeValues.clear();
    auto themes = mClientInfoViewModel->getSupportedThemeTypes();
    auto currentTheme = mClientInfoViewModel->getCurrentThemeType();
    for (size_t i = 0; i < themes.size(); ++i)
    {
        mThemeValues.push_back(static_cast<int>(themes[i]));
        mSupportedThemes.append(themeTypeToDisplayString(static_cast<int>(themes[i])));
        if (themes[i] == currentTheme)
            mCurrentThemeIndex = static_cast<int>(i);
    }
    emit supportedThemesChanged();
    emit currentThemeIndexChanged();

    // Languages
    mSupportedLanguages.clear();
    mLanguageValues.clear();
    auto languages = mClientInfoViewModel->getSupportedLanguages();
    auto currentLang = mClientInfoViewModel->getApplicationLanguage();
    for (size_t i = 0; i < languages.size(); ++i)
    {
        mLanguageValues.push_back(static_cast<int>(languages[i]));
        mSupportedLanguages.append(languageTypeToDisplayString(static_cast<int>(languages[i])));
        if (languages[i] == currentLang)
            mCurrentLanguageIndex = static_cast<int>(i);
    }
    emit supportedLanguagesChanged();
    emit currentLanguageIndexChanged();
}

QString SettingsPageController::themeTypeToDisplayString(int themeType) const
{
    using ThemeType = commonHead::viewModels::model::ThemeType;
    switch (static_cast<ThemeType>(themeType))
    {
    case ThemeType::SystemDefault: return tr("System Default");
    case ThemeType::Dark:          return tr("Dark");
    case ThemeType::Light:         return tr("Light");
    default:                       return tr("Unknown");
    }
}

QString SettingsPageController::languageTypeToDisplayString(int languageType) const
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
