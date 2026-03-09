#include "PageViews/SettingsPage/include/AppearanceSettingsController.h"
#include "LoggerDefine/LoggerDefine.h"

#include <commonHead/viewModels/ClientInfoViewModel/IClientInfoViewModel.h>
#include <commonHead/viewModels/ClientInfoViewModel/ClientInfoModel.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>
#include <AppContext/AppContext.h>
#include <UIManager/IUIManagerProvider.h>
#include <UIResourceLoaderManager/IUIResourceLoaderManager.h>

AppearanceSettingsController::AppearanceSettingsController(QObject* parent)
    : UIViewController(parent)
{
    UIVIEW_LOG_DEBUG("create AppearanceSettingsController");
}

void AppearanceSettingsController::init()
{
    UIVIEW_LOG_DEBUG("AppearanceSettingsController::init");

    m_clientInfoViewModel = getAppContext()->getViewModelFactory()->createClientInfoViewModelInstance();
    m_clientInfoViewModel->initViewModel();

    buildThemeData();

    UIVIEW_LOG_DEBUG("AppearanceSettingsController::init done");
}

QStringList AppearanceSettingsController::getSupportedThemes() const
{
    return m_supportedThemes;
}

int AppearanceSettingsController::getCurrentThemeIndex() const
{
    return m_currentThemeIndex;
}

void AppearanceSettingsController::setTheme(int index)
{
    if (index < 0 || index >= static_cast<int>(m_themeValues.size()))
        return;

    auto themeType = static_cast<commonHead::viewModels::model::ThemeType>(m_themeValues[static_cast<size_t>(index)]);
    UIVIEW_LOG_DEBUG("setTheme index:" << index << " themeType:" << m_themeValues[static_cast<size_t>(index)]);

    m_currentThemeIndex = index;
    emit currentThemeIndexChanged();

    m_clientInfoViewModel->setCurrentThemeType(themeType);
    getAppContext()->getManagerProvider()->getUIResourceLoaderManager()->notifyThemeChanged();
}

void AppearanceSettingsController::buildThemeData()
{
    if (!m_clientInfoViewModel)
        return;

    m_supportedThemes.clear();
    m_themeValues.clear();
    auto themes = m_clientInfoViewModel->getSupportedThemeTypes();
    auto currentTheme = m_clientInfoViewModel->getCurrentThemeType();
    for (size_t i = 0; i < themes.size(); ++i)
    {
        m_themeValues.push_back(static_cast<int>(themes[i]));
        m_supportedThemes.append(themeTypeToDisplayString(static_cast<int>(themes[i])));
        if (themes[i] == currentTheme)
            m_currentThemeIndex = static_cast<int>(i);
    }
    emit supportedThemesChanged();
    emit currentThemeIndexChanged();
}

QString AppearanceSettingsController::themeTypeToDisplayString(int themeType) const
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
