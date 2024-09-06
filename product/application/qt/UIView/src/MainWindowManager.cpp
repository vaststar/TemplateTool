#include "MainWindowManager.h"

#include <QObject>
#include <QGuiApplication>
#include <QQuickView>
#include <QQmlApplicationEngine>


#include <UICore/MainApplication.h>

#include "LoggerDefine.h"


/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class MainWindowManager::DataPrivate
{
public:
    DataPrivate(const MainWindowManager::ApplicationConfig& config);
    int runApp(){ return mainApp.exec();}
public:
    MainApplication mainApp;
};

MainWindowManager::DataPrivate::DataPrivate(const MainWindowManager::ApplicationConfig& config)
    : mainApp{ config.argc, config.argv }
{

}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
MainWindowManager::MainWindowManager(const MainWindowManager::ApplicationConfig& config)
    : mDataPrivate(std::make_unique<MainWindowManager::DataPrivate>(config))
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
    return mDataPrivate->runApp();
}

void MainWindowManager::createAndShowMainWindow()
{
    // QQmlApplicationEngine engine;

    UIVIEW_LOG_DEBUG("start load main qml");
    const QUrl url(QStringLiteral("qrc:/qt/qml/UIView/qml/MainWindow/MainWindow.qml"));
    QObject::connect(mDataPrivate->mainApp.mApplicationEngine, &QQmlApplicationEngine::objectCreated, &mDataPrivate->mainApp,[url, this](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
        {
            QCoreApplication::exit(-1);
        }
    }, Qt::QueuedConnection);
    mDataPrivate->mainApp.mApplicationEngine->load(url);

    UIVIEW_LOG_DEBUG("finish load main qml");

}