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
class MainWindowController : public UICore::CoreController
{
    Q_OBJECT
    Q_PROPERTY(QString mTitle READ getTitle NOTIFY titleChanged)
    Q_PROPERTY(QString mButtonText MEMBER mButtonText NOTIFY titleChanged)
    QML_ELEMENT
public:
    MainWindowController(QObject* parent =nullptr);
    virtual QString getControllerName() const override;

    QString getTitle() const;
    QString getButton() const;

    void initializeController(AppContext* appContext);

// private:
//     void showContactWindow()

signals:
    void titleChanged();
    void controllerInitialized();
public slots:
    void onContactListLoaded(ContactListViewController* contactListController);
    void openCamera();
private:
    QPointer<AppContext> mAppContext;
    QString mTitle;
    QString mButtonText;
    std::shared_ptr<commonHead::viewModels::IMainWindowViewModel> mMainViewModel;
};