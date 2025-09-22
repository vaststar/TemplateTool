#pragma once

#include <QString>
#include <QObject>
#include <QtQml>

#include <UTMenu/MenuItemModel.h>

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
    Q_PROPERTY(MenuItemModel* rootMenu READ rootMenu CONSTANT)
    Q_PROPERTY(QList<MenuItemModel*> listMenu READ listMenu CONSTANT)
    QML_ELEMENT
public:
    MainWindowMenuBarController(QObject* parent = nullptr);

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
protected:
    virtual void init() override;
private:
    std::shared_ptr<commonHead::viewModels::IMainWindowViewModel> mMainViewModel;
    MenuItemModel* mRootMenu{nullptr};
};