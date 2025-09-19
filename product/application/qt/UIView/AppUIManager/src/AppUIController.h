#pragma once

#include <QString>
#include <QObject>
#include <QtQml>
#include <UICore/CoreController.h>

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;

    namespace viewModels{
            class IAppUIViewModel;
    }
}

class AppContext;
class AppUIController : public UICore::CoreController
{
    Q_OBJECT
public:
    AppUIController(QObject* parent = nullptr);

    void runApp(AppContext* appContext);

signals:
    void controllerInitialized();
private:
    void initializeController(AppContext* appContext);
    void initializeUIClient();
    void startApp();
private:
    QPointer<AppContext> mAppContext;
    std::shared_ptr<commonHead::viewModels::IAppUIViewModel> mViewModel;
};