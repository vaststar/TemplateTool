#include "AppUIManager/include/AppUIManager.h"

#include <memory>

#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/ResourceLoader/IResourceLoader.h>

#include <UICore/CoreApplication.h>
#include <UICore/CoreQmlEngine.h>
#include <AppContext/AppContext.h>

#include <UIDataStruct/UIDataUtils.h>
#include <UIResourceLoader/UIResourceLoader.h>
#include <UIResourceStringLoader/UIResourceStringLoader.h>

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
private:
    void registerQmlTypes();
    void registerStringLoader();
private:
    std::unique_ptr<UICore::CoreApplication> mainApp;
    std::unique_ptr<UICore::CoreQmlEngine> mQmlEngine;
    std::unique_ptr<AppContext> mAppContext;
    commonHead::ICommonHeadFrameworkWPtr mCommonheadFramework;
};

AppUIManager::Impl::Impl(const AppUIManager::ApplicationConfig& config)
    : mainApp(std::make_unique<UICore::CoreApplication>( config.argc, config.argv ))
    , mQmlEngine(std::make_unique<UICore::CoreQmlEngine>())
    , mCommonheadFramework(config.commonHeadFramework)
    , mAppContext(std::make_unique<AppContext>(mainApp.get(), mQmlEngine.get(), config.commonHeadFramework))
{
    registerQmlTypes();
    registerStringLoader();
}

void AppUIManager::Impl::registerQmlTypes()
{
    UIDataUtils::registerMetaObject();   
    UIResouce::UIResourceLoader::registerMetaObject();
}

void AppUIManager::Impl::registerStringLoader()
{
    if (auto commonHeadFramework = mCommonheadFramework.lock())
    {
        if (auto resourceLoader = commonHeadFramework->getResourceLoader())
        {
            resourceLoader->setLocalizedStringLoader(UIResouce::UIResourceStringLoader::generateResourceStringLoader());
        }
    }
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
    controller->runApp(mImpl->getAppContext());

    return mImpl->runApp();
}