#include "AppUIManager/include/AppUIManager.h"

#include <memory>

#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>

#include <UICore/CoreApplication.h>
#include <UICore/CoreQmlEngine.h>
#include <AppContext/AppContext.h>

#include <UIDataStruct/UIDataUtils.h>
#include <UTComponent/UTComponent.h>

#include "LoggerDefine/LoggerDefine.h"
#include "UIViewBase/include/UIViewControllerInitializer.h"
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
private:
    void registerQmlTypes();
private:
    std::unique_ptr<UICore::CoreApplication> mainApp;
    std::unique_ptr<UICore::CoreQmlEngine> mQmlEngine;
    std::unique_ptr<AppContext> mAppContext;
    std::unique_ptr<UIViewControllerInitializer> mControllerInitializer;
};

AppUIManager::Impl::Impl(const AppUIManager::ApplicationConfig& config)
    : mainApp(std::make_unique<UICore::CoreApplication>( config.argc, config.argv ))
    , mQmlEngine(std::make_unique<UICore::CoreQmlEngine>())
    , mAppContext(std::make_unique<AppContext>(mainApp.get(), mQmlEngine.get(), config.commonHeadFramework))
    , mControllerInitializer(std::make_unique<UIViewControllerInitializer>(mAppContext.get()))
{
    registerQmlTypes();
}

void AppUIManager::Impl::registerQmlTypes()
{
    UIVIEW_LOG_DEBUG("");
    UIDataUtils::registerMetaObject();   
    UTComponent::registerUTComponent();
    mQmlEngine->rootContext()->setContextProperty("ControllerInitializer", mControllerInitializer.get());
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
    UIVIEW_LOG_INFO("run AppUIManager, address:" << this);
    auto controller = std::make_unique<AppUIController>();
    controller->runApp(mImpl->getAppContext());

    return mImpl->runApp();
}