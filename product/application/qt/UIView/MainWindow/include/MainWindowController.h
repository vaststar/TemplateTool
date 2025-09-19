#pragma once

#include <QString>
#include <QObject>
#include <QtQml>
#include <UICore/CoreController.h>

#include "UIViewBase/include/UIViewController.h"

//don't know why this is needed because the linux report unknown type of these controller
#include "ContactList/include/ContactListViewController.h"
#include "MainWindowMenuBar/include/MainWindowMenuBarController.h"
#include "MainWindowTitleBar/include/MainWindowTitleBarController.h"
#include "MainWindowFootBar/include/MainWindowFootBarController.h"
#include "MainWindowSideBar/include/MainWindowSideBarController.h"
#include "AppSystemTray/include/AppSystemTrayController.h"

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;

    namespace viewModels{
        class IMainWindowViewModel;
        class IMediaCameraViewModel;
    }
}

class AppContext;
// class ContactListViewController;
// class MainWindowMenuBarController;
// class MainWindowTitleBarController;
// class MainWindowFootBarController;
// class MainWindowSideBarController;
// class AppSystemTrayController;
// //forward declaration to use the poinnter in slots
// Q_DECLARE_OPAQUE_POINTER(ContactListViewController*)
// Q_DECLARE_OPAQUE_POINTER(MainWindowMenuBarController*)
// Q_DECLARE_OPAQUE_POINTER(MainWindowTitleBarController*)
// Q_DECLARE_OPAQUE_POINTER(MainWindowFootBarController*)
// Q_DECLARE_OPAQUE_POINTER(MainWindowSideBarController*)
// Q_DECLARE_OPAQUE_POINTER(AppSystemTrayController*);

class MainWindowController : public UIViewController
{
    Q_OBJECT
    Q_PROPERTY(QString title READ getTitle NOTIFY titleChanged)
    Q_PROPERTY(int height READ getHeight NOTIFY windowSizeChanged)
    Q_PROPERTY(int width READ getWidth NOTIFY windowSizeChanged)
    Q_PROPERTY(bool visible READ isVisible NOTIFY visibleChanged)
    QML_ELEMENT
public:
    MainWindowController(QObject* parent = nullptr);

    QString getTitle() const;
    int getHeight() const;
    int getWidth() const;

    void init();

    bool isVisible() const;
signals:
    void titleChanged();
    void windowSizeChanged();
    void controllerInitialized();
    void visibleChanged();
public slots:
    void onInitMenuBarController(MainWindowMenuBarController* menuBarController);
    void onContactListLoaded(ContactListViewController* contactListController);
    void onInitTitleBarController(MainWindowTitleBarController* titleBarController);
    void onInitFootBarController(MainWindowFootBarController* footBarController);
    void onInitSideBarController(MainWindowSideBarController* sideBarController);
    void onInitSystemTrayController(AppSystemTrayController* systemTrayController);
    void openCamera();
    void testFunc();
private:
    std::shared_ptr<commonHead::viewModels::IMainWindowViewModel> mMainViewModel;
};