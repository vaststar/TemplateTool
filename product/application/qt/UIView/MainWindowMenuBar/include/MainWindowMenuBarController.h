#pragma once

#include <QString>
#include <QObject>
#include <QtQml>
#include <QVariantList>

#include "UIViewBase/include/UIViewController.h"

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;

    namespace viewModels{
        class IMainWindowViewModel;
    }
}
namespace UILanguage{
    enum class LanguageType;
}

class AppContext;
class MainWindowMenuBarController : public UIViewController
{
    Q_OBJECT
    Q_PROPERTY(QVariantList menuModel READ menuModel NOTIFY menuModelChanged)
    QML_ELEMENT
public:
    MainWindowMenuBarController(QObject* parent = nullptr);

    QVariantList menuModel() const { return m_menuModel; }

    Q_INVOKABLE void handleMenuAction(const QString& action);

signals:
    void titleChanged();
    void controllerInitialized();
    void menuModelChanged();
public slots:
    void switchLanguage(UILanguage::LanguageType languageType);
protected:
    virtual void init() override;
private:
    void buildMenuModel();
    std::shared_ptr<commonHead::viewModels::IMainWindowViewModel> mMainViewModel;
    QVariantList m_menuModel;
};
