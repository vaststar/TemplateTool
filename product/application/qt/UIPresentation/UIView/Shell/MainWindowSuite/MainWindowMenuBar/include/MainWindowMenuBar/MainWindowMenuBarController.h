#pragma once

#include <QString>
#include <QObject>
#include <QtQml>
#include <QVariantList>

#include <UIViewCore/UIViewController.h>

namespace UILanguage{
    enum class LanguageType;
}

class MainWindowMenuBarController : public UIViewController
{
    Q_OBJECT
    Q_PROPERTY(QVariantList menuModel READ menuModel NOTIFY menuModelChanged)
    QML_ELEMENT
public:
    MainWindowMenuBarController(QObject* parent = nullptr);
    ~MainWindowMenuBarController() override;

    QVariantList menuModel() const { return m_menuModel; }

    Q_INVOKABLE void handleMenuAction(const QString& action);

signals:
    void menuModelChanged();
public slots:
    void switchLanguage(UILanguage::LanguageType languageType);
protected:
    void init() override;
private:
    void buildMenuModel();
    QVariantList m_menuModel;
};
