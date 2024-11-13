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
namespace UIManager{
    enum class LanguageType;
}

class AppContext;
class ContactListViewController;
class AppMenuBarController : public UICore::CoreController
{
    Q_OBJECT
    QML_ELEMENT
public:
    AppMenuBarController(QObject* parent = nullptr);
    virtual QString getControllerName() const override;

    void initializeController(AppContext* appContext);

signals:
    void titleChanged();
    void controllerInitialized();
public slots:
    void onContactListLoaded(ContactListViewController* contactListController);
    void switchLanguage(UIManager::LanguageType languageType);
private:
    QPointer<AppContext> mAppContext;
    std::shared_ptr<commonHead::viewModels::IMainWindowViewModel> mMainViewModel;
};