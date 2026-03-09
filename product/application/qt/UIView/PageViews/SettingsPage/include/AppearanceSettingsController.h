#pragma once

#include <QObject>
#include <QtQml>
#include "UIViewBase/include/UIViewController.h"

namespace commonHead::viewModels {
    class IClientInfoViewModel;
}

class AppearanceSettingsController : public UIViewController
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QStringList supportedThemes READ getSupportedThemes NOTIFY supportedThemesChanged)
    Q_PROPERTY(int currentThemeIndex READ getCurrentThemeIndex NOTIFY currentThemeIndexChanged)

public:
    explicit AppearanceSettingsController(QObject* parent = nullptr);

    QStringList getSupportedThemes() const;
    int getCurrentThemeIndex() const;

    Q_INVOKABLE void setTheme(int index);

protected:
    void init() override;

signals:
    void supportedThemesChanged();
    void currentThemeIndexChanged();

private:
    void buildThemeData();
    QString themeTypeToDisplayString(int themeType) const;

    std::shared_ptr<commonHead::viewModels::IClientInfoViewModel> m_clientInfoViewModel;
    QStringList m_supportedThemes;
    std::vector<int> m_themeValues;
    int m_currentThemeIndex = 0;
};
