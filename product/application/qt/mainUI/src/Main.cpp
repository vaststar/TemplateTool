#include "Main/Main.h"

#include <QGuiApplication>
#include <QQuickView>
#include <QQmlApplicationEngine>

#include <ucf/CoreFramework/ICoreFramework.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>

#include "ClientGlobal/ClientGlobal.h"
#include "Main/LoggerDefine.h"


Main::Main()
{
}

Main::~Main()
{
    MAINUI_LOG_DEBUG("delete Main");
}

void Main::initMain(int argc, char *argv[])
{
    AppRunner::AppLogConfig logConfig{
        "applogs",
        "temeplateTool",
        MasterLogUtil::ALL_LOG_LEVEL,
        180,
        50 * 1024 * 1024
    };
    AppRunner::ApplicationConfig appConfig{logConfig};
    mDependencies = AppRunner::initAppDependencies(appConfig);

    MAINUI_LOG_DEBUG("initAppDependencies done");
    ClientGlobal::getInstance()->setCommonHeadFramework(std::weak_ptr(mDependencies.commonHeadFramework));
    MAINUI_LOG_DEBUG("init Main done");
}

int Main::runMain(int argc, char *argv[])
{
    initMain(argc, argv);
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    MAINUI_LOG_DEBUG("start load main qml");
    const QUrl url(QStringLiteral("qrc:/qt/qml/UIView/qml/MainWindow/MainWindow.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app, [url, this](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
        {
            QCoreApplication::exit(-1);
        }
    }, Qt::QueuedConnection);
    engine.load(url);
    MAINUI_LOG_DEBUG("finish load main qml");

    int appResult = app.exec();
    MAINUI_LOG_DEBUG("quit App");
    onExitApp();
    return appResult;
}

void Main::onExitApp()
{
    MAINUI_LOG_DEBUG("exit app");
    mDependencies.coreFramework->exitCoreFramework();
}