#include "AppUIManager/include/AppUIManager.h"

#include <memory>

#include <UICore/CoreApplication.h>
#include <UICore/CoreQmlEngine.h>
#include <AppContext/AppContext.h>
#include <UIManager/TranslatorManager.h>

#include "LoggerDefine/LoggerDefine.h"
#include "AppUIController.h"


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

    AppContext* getAppContext() const { return mAppContext.get();}
    void registerQmlTypes();
private:
    std::unique_ptr<UICore::CoreApplication> mainApp;
    std::unique_ptr<UICore::CoreQmlEngine> mQmlEngine;
    std::unique_ptr<AppContext> mAppContext;
};

AppUIManager::Impl::Impl(const AppUIManager::ApplicationConfig& config)
    : mainApp(std::make_unique<UICore::CoreApplication>( config.argc, config.argv ))
    , mQmlEngine(std::make_unique<UICore::CoreQmlEngine>())
    , mAppContext(std::make_unique<AppContext>(mainApp.get(), mQmlEngine.get(), config.commonHeadFramework))
{
    registerQmlTypes();
}

void AppUIManager::Impl::registerQmlTypes()
{
    qmlRegisterUncreatableMetaObject(
		        UIManager::staticMetaObject, // The meta-object of the namespace
		        "UIManager",                 // The URI or module name
		        1, 0,                          // Version
		        "UIManager",                 // The name used in QML
		        "Access to enums only"         // Error message for attempting to create an instance
		    );
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
    auto controller = std::make_unique<AppUIController>();
    controller->initializeController(mImpl->getAppContext());
    controller->startApp();

    return mImpl->runApp();
}