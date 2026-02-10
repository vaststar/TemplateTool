#include "AppUIManager/include/AppUIManager.h"

#include <memory>

#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>

#include <UIAppCore/UIApplication.h>
#include <UIAppCore/UIQmlEngine.h>
#include <AppContext/AppContext.h>

#include <UIManager/UILanguage.h>
#include <UTComponent/UTComponent.h>

#include "LoggerDefine/LoggerDefine.h"
// #include "UIViewBase/include/UIViewControllerInitializer.h"
#include "AppUIController.h"
#include "ViewModelSingalEmitter/ViewModelTypeRegistry.h"

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
    std::unique_ptr<UIAppCore::UIApplication> mainApp;
    std::unique_ptr<UIAppCore::UIQmlEngine> mQmlEngine;
    std::unique_ptr<AppContext> mAppContext;
    // std::unique_ptr<UIViewControllerInitializer> mControllerInitializer;
};

AppUIManager::Impl::Impl(const AppUIManager::ApplicationConfig& config)
    : mainApp(std::make_unique<UIAppCore::UIApplication>( config.argc, config.argv ))
    , mQmlEngine(std::make_unique<UIAppCore::UIQmlEngine>())
    , mAppContext(std::make_unique<AppContext>(mainApp.get(), mQmlEngine.get(), config.commonHeadFramework))
    // , mControllerInitializer(std::make_unique<UIViewControllerInitializer>(mAppContext.get()))
{
    UIVIEW_LOG_INFO("===========================================");
    UIVIEW_LOG_INFO("===========create AppUIManagerImpl=========");
    UIVIEW_LOG_INFO("Qt Version: " << qVersion());
    registerQmlTypes();
    UIVIEW_LOG_INFO("===========create AppUIManagerImpl done====");
    UIVIEW_LOG_INFO("===========================================");
}

void AppUIManager::Impl::registerQmlTypes()
{
    UIVIEW_LOG_DEBUG("");
    UILanguage::registerMetaObject();   
    UTComponent::registerUTComponent();
    UIViewModelTypeRegistry::registerTypes();
    // mQmlEngine->rootContext()->setContextProperty("ControllerInitializer", mControllerInitializer.get());
    UIVIEW_LOG_DEBUG("done");
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish Impl Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
AppUIManager::AppUIManager(const AppUIManager::ApplicationConfig& config)
    : mImpl(std::make_unique<AppUIManager::Impl>(config))
{
    UIVIEW_LOG_INFO("===========================================");
    UIVIEW_LOG_INFO("create AppUIManager, address:" << this);
    UIVIEW_LOG_INFO("============================================");
}

AppUIManager::~AppUIManager()
{
    UIVIEW_LOG_INFO("exit AppUIManager, address:" << this);
}

int AppUIManager::runApp()
{
    UIVIEW_LOG_INFO("===========================================");
    UIVIEW_LOG_INFO("run AppUIManager, address:" << this << ", appContext's address: " << mImpl->getAppContext());
    auto controller = std::make_unique<AppUIController>(mImpl->getAppContext());
    UIVIEW_LOG_INFO("=======start UI with AppUIController======");
    controller->startApp();
    UIVIEW_LOG_INFO("=====start UI with AppUIController done===");
    UIVIEW_LOG_INFO("===========================================");

    UIVIEW_LOG_INFO("===========================================");
    UIVIEW_LOG_INFO("==start runApp in AppUIController==========");
    UIVIEW_LOG_INFO("===========================================");
    int res = mImpl->runApp();
    UIVIEW_LOG_INFO("===========================================");
    UIVIEW_LOG_INFO("======quit mainApp in AppUIController======");
    UIVIEW_LOG_INFO("===========================================");
    return res;
}