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
    MainWindowController(QObject* parent = nullptr);
    virtual QString getControllerName() const override;

    QString getTitle() const;

    void initializeController(QPointer<AppContext> appContext);

signals:
    void titleChanged();
    void controllerInitialized();
public slots:
    void onContactListLoaded(ContactListViewController* contactListController);
    void onInitMenuBarController(AppMenuBarController* menuBarController);
    void openCamera();
    void testFunc();
private:
    QPointer<AppContext> mAppContext;
    std::shared_ptr<commonHead::viewModels::IMainWindowViewModel> mMainViewModel;
};