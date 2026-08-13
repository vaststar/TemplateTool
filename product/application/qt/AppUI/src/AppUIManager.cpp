#include "AppUIManager/AppUIManager.h"

#include <memory>

#include <QGuiApplication>
#include <QScreen>
#include <QSysInfo>
#include <QCoreApplication>

#include <commonhead/CommonHeadFramework/ICommonHeadFramework.h>

#include <UIAppCore/UIApplication.h>
#include <UIAppCore/UIQmlEngine.h>
#include <AppContext/AppContext.h>

#include <TranslatorManager/UILanguage.h>
#include <UTComponent/UTComponent.h>
#include <UTComposite/UTComposite.h>
#include <UIView/UIViewModule.h>

#include "LoggerDefine.h"
#include "AppUIController.h"
#include <UIViewModelSignalBridge/RegisterViewModelMetaTypes.h>

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
};

AppUIManager::Impl::Impl(const AppUIManager::ApplicationConfig& config)
    : mainApp(std::make_unique<UIAppCore::UIApplication>( config.argc, config.argv ))
    , mQmlEngine(std::make_unique<UIAppCore::UIQmlEngine>())
    , mAppContext(std::make_unique<AppContext>(mainApp.get(), mQmlEngine.get(), config.commonHeadFramework))
{
    APPUI_LOG_INFO("===========================================");
    APPUI_LOG_INFO("===========create AppUIManagerImpl=========");
    APPUI_LOG_INFO("Qt Version: " << qVersion());
    APPUI_LOG_INFO("Qt Build ABI: " << QSysInfo::buildAbi().toStdString());
    APPUI_LOG_INFO("Qt Platform Plugin: " << QGuiApplication::platformName().toStdString());
    APPUI_LOG_INFO("Application PID: " << QCoreApplication::applicationPid());
    APPUI_LOG_INFO("Device Pixel Ratio: " << qApp->devicePixelRatio());
    const auto screens = QGuiApplication::screens();
    APPUI_LOG_INFO("Screen Count: " << screens.size());
    for (const QScreen* screen : screens)
    {
        if (!screen)
        {
            continue;
        }
        const QRect geometry = screen->geometry();
        APPUI_LOG_INFO("Screen [" << screen->name().toStdString() << "]"
            << " resolution: " << geometry.width() << "x" << geometry.height()
            << ", dpr: " << screen->devicePixelRatio()
            << ", logicalDPI: " << screen->logicalDotsPerInch()
            << ", refreshRate: " << screen->refreshRate() << "Hz");
    }
    registerQmlTypes();
    APPUI_LOG_INFO("===========create AppUIManagerImpl done====");
    APPUI_LOG_INFO("===========================================");
}

void AppUIManager::Impl::registerQmlTypes()
{
    APPUI_LOG_DEBUG("");
    UIViewModule::ensureLoaded();
    UIViewModelSignalBridge::registerAllViewModelMetaTypes();
    UILanguage::registerMetaObject();
    UTComponent::registerUTComponent();
    UTComposite::registerUTComposite();
    APPUI_LOG_DEBUG("done");
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish Impl Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
AppUIManager::AppUIManager(const AppUIManager::ApplicationConfig& config)
    : mImpl(std::make_unique<AppUIManager::Impl>(config))
{
    APPUI_LOG_INFO("===========================================");
    APPUI_LOG_INFO("create AppUIManager, address:" << this);
    APPUI_LOG_INFO("============================================");
}

AppUIManager::~AppUIManager()
{
    APPUI_LOG_INFO("exit AppUIManager, address:" << this);
}

int AppUIManager::runApp()
{
    APPUI_LOG_INFO("===========================================");
    APPUI_LOG_INFO("run AppUIManager, address:" << this << ", appContext's address: " << mImpl->getAppContext());
    auto controller = std::make_unique<AppUIController>();
    APPUI_LOG_INFO("=======start UI with AppUIController======");
    controller->start(mImpl->getAppContext());
    APPUI_LOG_INFO("=====start UI with AppUIController done===");
    APPUI_LOG_INFO("===========================================");

    APPUI_LOG_INFO("===========================================");
    APPUI_LOG_INFO("==start runApp in AppUIController==========");
    APPUI_LOG_INFO("===========================================");
    int res = mImpl->runApp();
    APPUI_LOG_INFO("===========================================");
    APPUI_LOG_INFO("======quit mainApp in AppUIController======");
    APPUI_LOG_INFO("===========================================");
    return res;
}
