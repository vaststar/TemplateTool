#include "AppUIManager/include/AppUIManager.h"

#include <memory>
#include <QObject>
#include <QGuiApplication>
#include <QQuickView>
#include <QQmlApplicationEngine>


#include <UICore/CoreApplication.h>
#include <UICore/CoreQmlApplicationEngine.h>
#include <UICore/CoreContext.h>
#include <UICore/CoreViewFactory.h>
#include <UICore/CoreViewModelFactory.h>

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

    void initAppContext(const AppUIManager::ApplicationConfig& config);

    const std::unique_ptr<CoreContext>& getAppContext() const{ return mAppContext;}
public:
    std::unique_ptr<CoreApplication> mainApp;
    std::unique_ptr<CoreContext> mAppContext;
};

AppUIManager::Impl::Impl(const AppUIManager::ApplicationConfig& config)
    : mainApp(std::make_unique<CoreApplication>( config.argc, config.argv ))
{
    initAppContext(config);
}

void AppUIManager::Impl::initAppContext(const AppUIManager::ApplicationConfig& config)
{
    auto viewModelFactory = std::make_unique<CoreViewModelFactory>(config.commonHeadFramework);
    auto qmlEngine = std::make_unique<CoreQmlApplicationEngine>();
    auto viewFactory = std::make_unique<CoreViewFactory>(std::move(qmlEngine));
    mAppContext = std::make_unique<CoreContext>(std::move(viewModelFactory),std::move(viewFactory));
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
            mainController->initializeController(mImpl->getAppContext().get());
        }
    });
    UIVIEW_LOG_DEBUG("finish load main qml");

}