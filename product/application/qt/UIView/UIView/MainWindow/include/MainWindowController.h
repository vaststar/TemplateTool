#pragma once

#include <QString>
#include <QObject>
#include <QtQml>
#include <UICore/CoreController.h>

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;

    namespace viewModels{
        class IMainWindowViewModel;
    }
}

class AppContext;
class ContactListViewController;
class AppMenuBarController;
class MainWindowController : public UICore::CoreController
{
    Q_OBJECT
    Q_PROPERTY(QString title READ getTitle NOTIFY titleChanged)
    QML_ELEMENT
public:
    MainWindowController(QPointer<AppContext> appContext, QObject* parent = nullptr);
    virtual QString getControllerName() const override;

    QString getTitle() const;

    void startAppWindow();

signals:
    void titleChanged();
    void controllerInitialized();
    void showMenuBar(AppMenuBarController* menuBarController);
public slots:
    // void onContactListLoaded(ContactListViewController* contactListController);
    void onInitMenuBarController(AppMenuBarController* menuBarController);
    void openCamera();
    void testFunc();

private:
    void initializeController(QPointer<AppContext> appContext);
private:
    const QPointer<AppContext> mAppContext;
    std::shared_ptr<commonHead::viewModels::IMainWindowViewModel> mMainViewModel;
};