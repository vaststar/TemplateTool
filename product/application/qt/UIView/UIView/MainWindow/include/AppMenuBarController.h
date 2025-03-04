#pragma once

#include <QString>
#include <QObject>
#include <QtQml>
#include <UICore/CoreController.h>
#include <UTMenu/MenuItemModel.h>

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
class ContactListViewController;
class AppMenuBarController : public UICore::CoreController
{
    Q_OBJECT
    Q_PROPERTY(MenuItemModel* rootMenu READ rootMenu CONSTANT)
    Q_PROPERTY(QList<MenuItemModel*> listMenu READ listMenu CONSTANT)
    QML_ELEMENT
public:
    AppMenuBarController(QObject* parent = nullptr);
    virtual QString getControllerName() const override;

    void initializeController(QPointer<AppContext> appContext);

    void createMenu();
    MenuItemModel* rootMenu() const{return mRootMenu;}
    QList<MenuItemModel*> listMenu() const {return mRootMenu->subItems();}
signals:
    void titleChanged();
    void controllerInitialized();
public slots:
    void switchLanguage(UILanguage::LanguageType languageType);
private slots:
    void onMenuItemTriggered(int itemIndex);
private:
    QPointer<AppContext> mAppContext;
    std::shared_ptr<commonHead::viewModels::IMainWindowViewModel> mMainViewModel;
    MenuItemModel* mRootMenu{nullptr};
};