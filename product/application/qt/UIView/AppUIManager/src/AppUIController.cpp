#include "AppUIController.h"

#include <UIFabrication/ViewModelFactory.h>
#include <UIFabrication/UIViewFactory.h>
#include <AppContext/AppContext.h>

#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>

#include "LoggerDefine/LoggerDefine.h"
#include "MainWindow/include/MainWindowController.h"


AppUIController::AppUIController(QObject* parent)
    : CoreController(parent)
    , mAppContext(nullptr)
{
    UIVIEW_LOG_DEBUG("create AppUIController");
}

QString AppUIController::getControllerName() const
{
    return "AppUIController";
}


void AppUIController::initializeController(AppContext* appContext)
{
    mAppContext = appContext;
    assert(mAppContext);

    emit controllerInitialized();
}

void AppUIController::startApp()
{
    //load db to see if we need show sign in/up or main window
    
    UIVIEW_LOG_DEBUG("start load main qml");

    mAppContext->getViewFactory()->loadQmlWindow(QStringLiteral("UIView/MainWindow/qml/MainWindow.qml"), "MainWindowController", [this](auto controller){
        if (auto mainController = dynamic_cast<MainWindowController*>(controller))
        {
            // mImpl->getAppContext()->getViewFactory()->installTranslation({});
            mainController->initializeController(mAppContext);
        }
    });
    UIVIEW_LOG_DEBUG("finish load main qml");

}