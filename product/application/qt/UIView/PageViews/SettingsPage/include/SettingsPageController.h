#pragma once

#include <QObject>
#include <QtQml>
#include "UIViewBase/include/UIViewController.h"
#include "SettingsNavModel.h"

namespace commonHead::viewModels {
    class IClientInfoViewModel;
}

class SettingsPageController : public UIViewController
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(SettingsNavModel* navModel READ getNavModel NOTIFY navModelChanged)
    Q_PROPERTY(QStringList supportedThemes READ getSupportedThemes NOTIFY supportedThemesChanged)
    Q_PROPERTY(QStringList supportedLanguages READ getSupportedLanguages NOTIFY supportedLanguagesChanged)
    Q_PROPERTY(int currentThemeIndex READ getCurrentThemeIndex NOTIFY currentThemeIndexChanged)
    Q_PROPERTY(int currentLanguageIndex READ getCurrentLanguageIndex NOTIFY currentLanguageIndexChanged)

public:
    explicit SettingsPageController(QObject* parent = nullptr);

    SettingsNavModel* getNavModel() const;
    QStringList getSupportedThemes() const;
    QStringList getSupportedLanguages() const;
    int getCurrentThemeIndex() const;
    int getCurrentLanguageIndex() const;

    Q_INVOKABLE void setTheme(int index);
    Q_INVOKABLE void setLanguage(int index);

protected:
    void init() override;

signals:
    void navModelChanged();
    void supportedThemesChanged();
    void supportedLanguagesChanged();
    void currentThemeIndexChanged();
    void currentLanguageIndexChanged();

private:
    void buildNavModel();
    void buildThemeAndLanguageData();
    QString themeTypeToDisplayString(int themeType) const;
    QString languageTypeToDisplayString(int languageType) const;

private:
    std::shared_ptr<commonHead::viewModels::IClientInfoViewModel> mClientInfoViewModel;
    SettingsNavModel* mNavModel = nullptr;

    QStringList mSupportedThemes;
    QStringList mSupportedLanguages;
    std::vector<int> mThemeValues;    // 存储实际 ThemeType 枚举值
    std::vector<int> mLanguageValues; // 存储实际 LanguageType 枚举值
    int mCurrentThemeIndex = 0;
    int mCurrentLanguageIndex = 0;
};
