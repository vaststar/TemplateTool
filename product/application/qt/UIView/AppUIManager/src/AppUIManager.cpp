#include "AppUIManager/include/AppUIManager.h"

#include <memory>
#include <QObject>
#include <QGuiApplication>
#include <QQuickView>
#include <QQmlApplicationEngine>


#include <UICore/CoreApplication.h>
#include <UICore/CoreQmlEngine.h>
#include <AppContext/AppContext.h>
#include <UIFabrication/UIViewFactory.h>
#include <UIFabrication/ViewModelFactory.h>

#include "LoggerDefine/LoggerDefine.h"
#include "MainWindow/include/MainWindowController.h"


/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start Impl Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class AppUIManager::Impl
{
public:
    Impl(const AppUIManager::ApplicationConfig& config);
    int runApp(){ return mainApp->exec();}

    const std::unique_ptr<AppContext>& getAppContext() const{ return mAppContext;}
public:
    std::unique_ptr<UICore::CoreApplication> mainApp;
    std::unique_ptr<UICore::CoreQmlEngine> mQmlEngine;
    std::unique_ptr<AppContext> mAppContext;
};

AppUIManager::Impl::Impl(const AppUIManager::ApplicationConfig& config)
    : mainApp(std::make_unique<UICore::CoreApplication>( config.argc, config.argv ))
    , mQmlEngine(std::make_unique<UICore::CoreQmlEngine>())
    , mAppContext(std::make_unique<AppContext>(mainApp.get(), mQmlEngine.get(), config.commonHeadFramework))
{
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish Impl Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
AppUIManager::AppUIManager(const AppUIManager::ApplicationConfig& config)
    : mImpl(std::make_unique<AppUIManager::Impl>(config))
{
    UIVIEW_LOG_INFO("create AppUIManager, address:" << this);
}

AppUIManager::~AppUIManager()
{
    UIVIEW_LOG_INFO("exit AppUIManager, address:" << this);
}

int AppUIManager::runApp()
{
    createAndShowMainWindow();

    return mImpl->runApp();
}

void AppUIManager::createAndShowMainWindow()
{

    UIVIEW_LOG_DEBUG("start load main qml");

    mImpl->getAppContext()->getViewFactory()->loadQmlWindow(QStringLiteral("UIView/MainWindow/qml/MainWindow.qml"), "MainWindowController", [this](auto controller){
        if (auto mainController = dynamic_cast<MainWindowController*>(controller))
        {
            // mImpl->getAppContext()->getViewFactory()->installTranslation({});
            mainController->initializeController(mImpl->getAppContext().get());
        }
    });
    UIVIEW_LOG_DEBUG("finish load main qml");

}