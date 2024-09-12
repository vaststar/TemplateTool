#include "MainWindowManager.h"

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

#include "LoggerDefine.h"
#include "MainWindowController.h"
#include "TestController.h"


/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start Impl Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class MainWindowManager::Impl
{
public:
    Impl(const MainWindowManager::ApplicationConfig& config);
    int runApp(){ return mainApp->exec();}

    void initAppContext(const MainWindowManager::ApplicationConfig& config);

    const std::unique_ptr<CoreContext>& getAppContext() const{ return mAppContext;}
public:
    std::unique_ptr<CoreApplication> mainApp;
    std::unique_ptr<CoreContext> mAppContext;
};

MainWindowManager::Impl::Impl(const MainWindowManager::ApplicationConfig& config)
    : mainApp(std::make_unique<CoreApplication>( config.argc, config.argv ))
{
    initAppContext(config);
}

void MainWindowManager::Impl::initAppContext(const MainWindowManager::ApplicationConfig& config)
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
MainWindowManager::MainWindowManager(const MainWindowManager::ApplicationConfig& config)
    : mImpl(std::make_unique<MainWindowManager::Impl>(config))
{
    
}

MainWindowManager::~MainWindowManager()
{

}

int MainWindowManager::runApp()
{
    createAndShowMainWindow();
    // QQuickView view;
    // view.setSource(QStringLiteral("qrc:/qt/qml/UIView/qml/MainWindow/MainWindow.qml"));
    // view.show();
    // QQuickView view;
    // view.setSource(QStringLiteral("qrc:/qt/qml/UIView/qml/testUI/testUI.qml"));
    // view.show();


    // MainWindowController *mainController = new MainWindowController(const CoreContext& mAppContext);
    // mainController->showMainWindow();

    return mImpl->runApp();
}

void MainWindowManager::createAndShowMainWindow()
{

    UIVIEW_LOG_DEBUG("start load main qml");
    // const QUrl url(QStringLiteral("qrc:/qt/qml/UIView/qml/MainWindow/MainWindow.qml"));

    mImpl->getAppContext()->getViewFactory()->loadQmlWindow(QStringLiteral("qrc:/qt/qml/UIView/qml/MainWindow/MainWindow.qml"),[this](auto controller){
        if (auto mainController = dynamic_cast<MainWindowController*>(controller))
        {
            mainController->initializeController(mImpl->getAppContext().get());
        }
    });

    //mImpl->getAppContext()->getViewFactory()->loadQmlWindow(QStringLiteral("qrc:/qt/qml/UIView/qml/testUI/testWindow.qml"));

    //QObject* rootObject = mImpl->getAppContext()->getViewFactory()->getQmlEngine()->rootObjects()[0];
    // mImpl->getAppContext()->getViewFactory()->loadQmlWindow(QStringLiteral("qrc:/qt/qml/UIView/qml/testUI/testWindow.qml"),[](auto controller){
    //     if (auto cc = dynamic_cast<TestController*>(controller))
    //     {
    //         cc->setName();
    //     }
    // });

    // mDataPrivate->mainApp.mApplicationEngine->load(url);
    // mDataPrivate->mainApp.mApplicationEngine->load(url);
    UIVIEW_LOG_DEBUG("finish load main qml");

}