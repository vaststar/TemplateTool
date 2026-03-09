#pragma once

#include <QObject>
#include <QtQml>
#include "UIViewBase/include/UIViewController.h"

namespace commonHead::viewModels {
    class IClientInfoViewModel;
}

class LanguageSettingsController : public UIViewController
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QStringList supportedLanguages READ getSupportedLanguages NOTIFY supportedLanguagesChanged)
    Q_PROPERTY(int currentLanguageIndex READ getCurrentLanguageIndex NOTIFY currentLanguageIndexChanged)

public:
    explicit LanguageSettingsController(QObject* parent = nullptr);

    QStringList getSupportedLanguages() const;
    int getCurrentLanguageIndex() const;

    Q_INVOKABLE void setLanguage(int index);

protected:
    void init() override;

signals:
    void supportedLanguagesChanged();
    void currentLanguageIndexChanged();

private:
    void buildLanguageData();
    QString languageTypeToDisplayString(int languageType) const;

    std::shared_ptr<commonHead::viewModels::IClientInfoViewModel> m_clientInfoViewModel;
    QStringList m_supportedLanguages;
    std::vector<int> m_languageValues;
    int m_currentLanguageIndex = 0;
};
